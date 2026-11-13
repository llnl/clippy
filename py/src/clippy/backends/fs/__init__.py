"""Filesystem backend for clippy."""

from __future__ import annotations

import json
import logging
import os
import pathlib
import stat
import sys
from concurrent.futures import ThreadPoolExecutor
from subprocess import CalledProcessError
from typing import Any

from ... import constants as clippy_constants
from ...clippy_types import CLIPPY_CONFIG
from ...error import (
    ClippyConfigurationError,
    ClippyInvalidKeypathError,
    ClippyTypeError,
    ClippyValidationError,
)
from ...keypaths import Keypath
from ...utils import flat_dict_to_nested
from ..version import _check_version
from .config import _fs_config_entries
from .execution import _help, _run, _validate

# create a fs-specific configuration.
cfg = CLIPPY_CONFIG(_fs_config_entries)

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
    if st.st_uid == uid and mode & stat.S_IXUSR:  # noqa: SIM114
        return True
    # Group permissions
    elif st.st_gid == gid and mode & stat.S_IXGRP:  # noqa: SIM114
        return True
    # Other permissions
    elif mode & stat.S_IXOTH:
        return True

    return False


def get_cfg() -> CLIPPY_CONFIG:
    """This is a mandatory function for all backends. It returns the backend-specific configuration."""
    return cfg


def classes(paths: list[str] | None = None) -> dict[str, Any]:
    """This is a mandatory function for all backends. It returns a dictionary of class name
    to the actual Class for all classes supported by the backend."""
    from ... import cfg as topcfg  # pylint: disable=import-outside-toplevel

    if paths is None:
        paths = cfg.get("fs_backend_paths")

    _classes = {}
    # iterate over all (filesystem) paths and for those that aren't explicitly excluded,
    # create the class based on the directory name and add methods based on the executables
    # within the directory.
    for path in paths:
        with os.scandir(path) as files:
            for f in files:
                if f.name in cfg.get("fs_exclude_paths"):
                    continue
                p = pathlib.Path(path, f)
                if os.path.isdir(p):
                    _cls = _create_class(f.name, path, topcfg)
                    _classes[f.name] = _cls
    return _classes


def add_backend_path(path: str) -> None:
    """Generate and expose classes found in one filesystem backend path."""
    from ... import _register_classes  # pylint: disable=import-outside-toplevel

    _register_classes(classes(paths=[path]))


def _init_instance(self):
    """Give a new instance empty state and its own copies of the class's initial keypaths."""
    self._state = {}
    for name, docstr in type(self)._initial_keypaths.items():
        setattr(self, name, Keypath(None, name, docstr))


def _create_class(name: str, path: str, topcfg: CLIPPY_CONFIG):
    """Given a name, a path, and a master configuration, create
    a class with the given name, and add methods based on the
    executable files within the class. Set convenience fields
    (_name, _path, _cfg) as well, and load class metadata from
    a meta.json file in each class directory. The meta.json
    file typically holds the class's docstring and any initial
    top-level keypaths as a dictionary of keypath: docstring."""
    metafile = pathlib.Path(path, name, clippy_constants.CLASS_META_FILE)
    meta = {}
    if metafile.exists():
        with open(metafile, encoding="utf-8") as json_file:
            meta = json.load(json_file)
    # pull the keypaths out since we don't want them in the class definition right now
    keypaths = meta.pop(clippy_constants.INITIAL_KEYPATH_KEY, {})
    # Keypaths are per-instance (created in _init_instance);
    # the class only records which ones each instance should get.
    meta["_initial_keypaths"] = keypaths
    # Default constructor for classes without an __init__ executable.
    # If there is one, _define_method replaces this and calls _init_instance itself.
    meta["__init__"] = _init_instance
    meta["_name"] = name
    meta["_path"] = path
    meta["_cfg"] = topcfg
    class_logger = logging.getLogger(topcfg.get("logname") + "." + name)
    class_logger.setLevel(topcfg.get("loglevel"))
    meta["logger"] = class_logger

    cls = type(name, (), meta)
    classpath = pathlib.Path(path, name)
    executables = []
    for file in os.scandir(classpath):
        fullpath = pathlib.Path(classpath, file)
        if _is_user_executable(fullpath):
            executables.append(fullpath)

    # Query executable metadata concurrently, then install methods in scan order.
    # Installing sequentially preserves deterministic behavior when multiple
    # executables expose the same method name.
    with ThreadPoolExecutor() as executor:
        definitions = [
            (executable, executor.submit(_get_method_definition, str(executable), class_logger))
            for executable in executables
        ]

        for executable, definition in definitions:
            try:
                method, docstring, args = definition.result()
            except ClippyConfigurationError as e:
                class_logger.warning("error processing %s: %s; ignoring", executable, e)
                continue

            _install_method(str(executable), cls, method, docstring, args)

    return cls


def _process_executable(executable: str, cls):
    """
    Stores the executable as a method of cls.
    details: The executable is queried for its description. The returned
             json file is parsed and converted to a type representing
             the executable's methods and state. The type is recorded in the
             symtable (by default globals()).
    """

    method, docstring, args = _get_method_definition(executable, cls.logger)
    _install_method(executable, cls, method, docstring, args)
    return cls


def _get_method_definition(executable: str, logger):
    """Query an executable and return the information needed to define its method."""
    logger.debug("processing executable %s", executable)
    try:
        j = _help(executable, {}, logger)

    except CalledProcessError as e:
        raise ClippyConfigurationError("Execution error " + e.stderr) from e

    # check to make sure we have the method name. This is so the executable can have
    # a different name than the actual method.
    if clippy_constants.METHODNAME_KEY not in j:
        raise ClippyConfigurationError("No method_name in " + executable)
    # check version
    if not _check_version(j):
        raise ClippyConfigurationError("Invalid version information in " + executable)

    docstring = j.get(clippy_constants.DOCSTRING_KEY, "")
    args: dict[str, dict] = j.get(clippy_constants.ARGS_KEY, {})  # this is now a dict.

    # Create a list of descriptions ordered by position (excluding position=-1)
    ordered_descs = []
    unordered_descs = []
    for arg, arginfo in sorted(args.items(), key=lambda x: x[1]["position"]):
        desc = f"{arg}: {arginfo['desc']}"
        if arginfo["position"] != -1:
            ordered_descs.append(desc)
        else:
            desc += f", default: {arginfo['default_val']}"
            unordered_descs.append(desc)

    if ordered_descs != []:
        docstring += "\nRequired Arguments:\n"
        for desc in ordered_descs:
            docstring += f"  {desc}\n"

    if unordered_descs != []:
        docstring += "\nOptional Arguments:\n"
        for desc in unordered_descs:
            docstring += f"  {desc}\n"

    # if we don't explicitly pass the method name, use the name of the exe.
    method = j.get(clippy_constants.METHODNAME_KEY, os.path.basename(executable))
    return method, docstring, args


def _install_method(executable: str, cls, method: str, docstring: str, args: dict[str, dict]) -> None:
    """Install a previously discovered executable method on a generated class."""
    if hasattr(cls, method) and not method.startswith("__"):
        cls.logger.warning(f"Overwriting existing method {method} for class {cls} with executable {executable}")
    _define_method(cls, method, executable, docstring, args)


def _define_method(cls, name: str, executable: str, docstr: str, arguments: dict[str, dict] | None):  # pylint: disable=too-complex
    """Defines a method on a given class."""
    if arguments is None:
        arguments = {}

    def m(self, *args, **kwargs):
        """
        Generic Method that calls an executable with specified arguments
        """

        # special cases for __init__
        # initialize _state and per-instance keypaths
        if name == "__init__":
            _init_instance(self)

        argdict = {}
        # statej  = {}

        # make json from args and state

        # .. add state
        # argdict[STATE_KEY] = self._state
        argdict[clippy_constants.STATE_KEY] = getattr(self, clippy_constants.STATE_KEY)

        # explicitly add loglevel to state if it doesn't already exist.
        argdict[clippy_constants.STATE_KEY][clippy_constants.LOGLEVEL_KEY] = self.logger.getEffectiveLevel()
        # Returns python integer that gets converted by cpp

        # ~ for key in statedesc:
        #     ~ statej[key] = getattr(self, key)

        # .. add positional arguments
        numpositionals = len(args)
        for argdesc in arguments:
            value = arguments[argdesc]
            if 0 <= value.get("position", -1) < numpositionals:
                argdict[argdesc] = args[value["position"]]

        # .. add keyword arguments
        argdict.update(kwargs)

        # validate
        valid, stderr = _validate(executable, argdict, self.logger)
        if not valid:
            raise ClippyValidationError(stderr)

        # call executable and create json output

        outj = _run(executable, argdict, self.logger)

        # if we have results that have keys that are in our
        # kwargs, let's update the kwarg references. Works
        # for lists and dicts only.
        for kw, kwval in kwargs.items():
            if kw in outj.get(clippy_constants.REFERENCE_KEY, {}):
                kwval.clear()
                if isinstance(kwval, dict):
                    kwval.update(outj[kw])
                elif isinstance(kwval, list):
                    kwval += outj[kw]
                else:
                    raise ClippyTypeError()

        # dump any output
        if clippy_constants.OUTPUT_KEY in outj:
            print(outj[clippy_constants.OUTPUT_KEY])
        # update state according to json output
        if clippy_constants.STATE_KEY in outj:
            setattr(self, clippy_constants.STATE_KEY, outj[clippy_constants.STATE_KEY])

        # update keypaths if necessary.
        if clippy_constants.KEYPATH_KEY in outj:
            d = flat_dict_to_nested(outj[clippy_constants.KEYPATH_KEY])
            for topkp, subpaths in d.items():
                if not hasattr(self, topkp):
                    raise ClippyInvalidKeypathError(f"keypath {topkp} not found in class; aborting")
                getattr(self, topkp)._import_from_dict(subpaths)

        # return result
        if outj.get(clippy_constants.SELF_KEY, False):
            return self
        return outj.get(clippy_constants.RETURN_KEY)

        # end of nested def m

    # Add a new member function with name and implementation m to the class cls
    # setattr(name, '__doc__', docstr)
    m.__doc__ = docstr
    setattr(cls, name, m)
