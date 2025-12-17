"""Monolith backend for clippy."""

from __future__ import annotations

import os
import stat
import json
import sys
import pathlib
import logging
import select
import subprocess
from typing import Any


from .execution import _validate, _run, _help
from ..version import _check_version
from ..serialization import ClippySerializable

from ... import constants
from . import constants as local_constants

from ...error import (
    ClippyConfigurationError,
    ClippyTypeError,
    ClippyValidationError,
    ClippyInvalidSelectorError,
)
from ...selectors import Selector
from ...utils import flat_dict_to_nested

from ...clippy_types import CLIPPY_CONFIG
from .config import _monolith_config_entries

# create a fs-specific configuration.
cfg = CLIPPY_CONFIG(_monolith_config_entries)

PATH = sys.path[0]


def _is_user_executable(path: pathlib.Path) -> bool:
    # Must be a regular file
    if not os.path.isfile(path):
        return False

    st = os.stat(path)
    mode = st.st_mode
    uid = os.getuid()
    gid = os.getgid()

    # Owner permissions
    if st.st_uid == uid and mode & stat.S_IXUSR:
        return True
    # Group permissions
    elif st.st_gid == gid and mode & stat.S_IXGRP:
        return True
    # Other permissions
    elif mode & stat.S_IXOTH:
        return True

    return False


def get_cfg() -> CLIPPY_CONFIG:
    """This is a mandatory function for all backends. It returns the backend-specific configuration."""
    return cfg


def send_cmd(
    p: subprocess.Popen, cmd: tuple[str, str], args: dict = {}
) -> tuple[dict, str]:
    """Sends a command with optional args and blocks until return of a dict of JSON-response output.
    The command is the first element in the tuple.
    """

    assert p.stdin is not None
    assert p.stdout is not None
    send_d = {"cmd": cmd[0]}
    send_d.update(args)

    send_j = json.dumps(send_d)
    p.stdin.write(send_j + "\n")
    p.stdin.flush()

    # Wait for response
    readable, _, _ = select.select(
        [p.stdout, p.stderr], [], [], local_constants.SELECT_TIMEOUT
    )
    if p.stderr in readable:  # return the stderr and exit.
        assert p.stderr is not None
        error = p.stderr.readline()
        return ({}, error)

    if p.stdout not in readable:
        return ({}, "Timeout waiting for response")

    # Read STATUS_START
    start_line = p.stdout.readline()
    try:
        start_status = json.loads(start_line)
        if start_status != local_constants.STATUS_START:
            return ({}, f"Expected STATUS_START, got: {start_status}")
    except json.JSONDecodeError:
        return ({}, f"Invalid JSON for STATUS_START: {start_line}")

    # Read zero or more dictionary lines until STATUS_END
    results = []
    while True:
        line = p.stdout.readline()
        if not line:
            return ({}, "Unexpected EOF while reading response")

        try:
            data = json.loads(line)
        except json.JSONDecodeError:
            return ({}, f"Invalid JSON in response: {line}")

        # Check if this is STATUS_END
        if data == local_constants.STATUS_END:
            break

        # Check if this is STATUS_UPDATE with a message
        if data.get(local_constants.STATUS_KEY) == "update":
            if "message" in data:
                print(data["message"])
            continue

        # Otherwise, it's a result dictionary
        results.append(data)

    # Package results into response dictionary
    recv_d = {local_constants.RESULTS_KEY: results}

    # Check for any stderr messages (non-blocking)
    readable, _, _ = select.select([p.stderr], [], [], 0)  # 0 timeout = immediate
    if p.stderr in readable:
        assert p.stderr is not None
        error = p.stderr.readline()
        return ({}, error)

    return (recv_d, "")


def classes() -> dict[str, Any]:
    """This is a mandatory function for all backends. It returns a dictionary of class name
    to the actual Class for all classes supported by the backend."""
    from ... import cfg as topcfg  # pylint: disable=import-outside-toplevel

    _classes = {}
    monolith_exe = cfg.get("monolith_exe")
    if not _is_user_executable(monolith_exe):
        raise ClippyConfigurationError("File is not executable: ", monolith_exe)

    p = subprocess.Popen(
        [
            monolith_exe,
        ],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,  # Decodes streams as text
    )

    # Wait for STATUS_READY from the subprocess
    assert p.stdout is not None
    assert p.stderr is not None
    readable, _, _ = select.select(
        [p.stdout, p.stderr], [], [], local_constants.READY_TIMEOUT
    )

    if p.stderr in readable:
        error = p.stderr.readline()
        raise ClippyConfigurationError(f"Error starting monolith: {error}")

    if p.stdout not in readable:
        raise ClippyConfigurationError(
            f"{monolith_exe} did not send ready status within {local_constants.READY_TIMEOUT} seconds"
        )

    status_line = p.stdout.readline()
    try:
        status = json.loads(status_line)
        if status != local_constants.STATUS_READY:
            raise ClippyConfigurationError(f"Expected STATUS_READY, got: {status}")
    except json.JSONDecodeError as e:
        raise ClippyConfigurationError(
            f"Invalid JSON from {monolith_exe}: {status_line}"
        ) from e

    path = pathlib.Path(monolith_exe).parent

    class_result, class_error = send_cmd(p, local_constants.CLASSES)

    class_dict = class_result.get(local_constants.RESULTS_KEY, {})
    # this is a dict of classname: classdata
    for class_name, class_data in class_dict:
        _classes[class_name] = _create_class(p, class_name, class_data, topcfg)

    return _classes


def _create_class(
    p: subprocess.Popen, class_name: str, class_data: dict, topcfg: CLIPPY_CONFIG
):
    """Given a dictionary of class data and a master configuration,
    create a class with the given name, and add methods based on the
    class_data. Set convenience fields (_name, _cfg) as well"""

    # pull the selectors out since we don't want them in the class definition right now
    selectors = class_data.pop(constants.INITIAL_SELECTOR_KEY, {})
    methods = class_data.pop(local_constants.CLASS_METHOD_KEY, {})
    class_data["_name"] = class_name
    class_data["_cfg"] = topcfg
    class_data["_p"] = p
    class_logger = logging.getLogger(topcfg.get("logname") + "." + class_name)
    class_logger.setLevel(topcfg.get("loglevel"))
    class_data["logger"] = class_logger

    cls = type(class_name, (ClippySerializable,), class_data)

    # add the methods
    for method, method_meta in methods:
        docstring = method_meta.get(constants.DOCSTRING_KEY, "")
        args = method_meta.get(constants.ARGS_KEY, {})

        if hasattr(cls, method) and not method.startswith("__"):
            assert hasattr(cls, "logger"), "Class must have a logger attribute"
            logger = getattr(cls, "logger")
            logger.warning(
                f"Overwriting existing method {method} for class {cls} with executable {executable}"
            )

        _define_method(cls, method, executable, docstring, args)

    # add the selectors
    # this should be in the meta.json file.
    for selector, docstr in selectors.items():
        class_logger.debug("adding %s to class", selector)
        setattr(cls, selector, Selector(None, selector, docstr))
    return cls


def _define_method(
    cls, name: str, docstr: str, arguments: list[str] | None
):  # pylint: disable=too-complex
    """Defines a method on a given class."""

    def m(self, *args, **kwargs):
        """
        Generic Method that calls an executable with specified arguments
        """

        # special cases for __init__
        # call the superclass to initialize the _state
        if name == "__init__":
            super(cls, self).__init__()

        argdict = {}
        # statej  = {}

        # make json from args and state

        # .. add state
        # argdict[STATE_KEY] = self._state
        argdict[constants.STATE_KEY] = getattr(self, constants.STATE_KEY)
        # ~ for key in statedesc:
        #     ~ statej[key] = getattr(self, key)

        # .. add positional arguments
        numpositionals = len(args)
        for argdesc in arguments:
            value = arguments[argdesc]
            if "position" in value:
                if 0 <= value["position"] < numpositionals:
                    argdict[argdesc] = args[value["position"]]

        # .. add keyword arguments
        argdict.update(kwargs)

        # call executable and create json output
        outj = _run(executable, argdict, self.logger)

        # if we have results that have keys that are in our
        # kwargs, let's update the kwarg references. Works
        # for lists and dicts only.
        for kw, kwval in kwargs.items():
            if kw in outj.get(constants.REFERENCE_KEY, {}):
                kwval.clear()
                if isinstance(kwval, dict):
                    kwval.update(outj[kw])
                elif isinstance(kwval, list):
                    kwval += outj[kw]
                else:
                    raise ClippyTypeError()

        # dump any output
        if constants.OUTPUT_KEY in outj:
            print(outj[constants.OUTPUT_KEY])
        # update state according to json output
        if constants.STATE_KEY in outj:
            setattr(self, constants.STATE_KEY, outj[constants.STATE_KEY])

        # update selectors if necessary.
        if constants.SELECTOR_KEY in outj:
            d = flat_dict_to_nested(outj[constants.SELECTOR_KEY])
            for topsel, subsels in d.items():
                if not hasattr(self, topsel):
                    raise ClippyInvalidSelectorError(
                        f"selector {topsel} not found in class; aborting"
                    )
                getattr(self, topsel)._import_from_dict(subsels)

        # return result
        if outj.get(constants.SELF_KEY, False):
            return self
        return outj.get(constants.RETURN_KEY)

        # end of nested def m

    # Add a new member function with name and implementation m to the class cls
    # setattr(name, '__doc__', docstr)
    m.__doc__ = docstr
    setattr(cls, name, m)
