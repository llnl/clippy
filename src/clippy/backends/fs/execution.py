"""
    Functions to execute backend programs.
"""

from __future__ import annotations
import json
import logging
import select

import subprocess
from ...clippy_types import AnyDict
from ... import cfg
from ...constants import OUTPUT_KEY
from .constants import (
    DRY_RUN_FLAG,
    HELP_FLAG,
    PROGRESS_INC_KEY,
    PROGRESS_SET_KEY,
    PROGRESS_START_KEY,
    PROGRESS_END_KEY,
)

from ...error import ClippyValidationError, ClippyBackendError
from ..serialization import encode_clippy_json, decode_clippy_json

try:
    from tqdm import tqdm

    _has_tqdm = True
except ImportError:
    _has_tqdm = False


def _stream_exec(
    cmd: list[str],
    submission_dict: AnyDict,
    logger: logging.Logger,
    validate: bool,
) -> tuple[AnyDict | None, str | None, int]:
    """
    Internal function.

    Executes the command specified with `execcmd` and
    passes `submission_dict` as JSON via STDIN.

    Logs debug messages with progress.
    Parses the object and returns a dictionary output.
    Returns the process result object, stderr, and the process return code.

    This function is used by _run and _validate. All options (pre_cmd and flags) should
    already be set.
    """

    logger.debug(f'Submission = {submission_dict}')
    # PP support passing objects
    # ~ cmd_stdin = json.dumps(submission_dict)
    cmd_stdin = json.dumps(submission_dict, default=encode_clippy_json)

    logger.debug("Calling %s with input %s", cmd, cmd_stdin)

    d = {}
    stderr_lines = []

    with subprocess.Popen(
        cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, encoding='utf8'
    ) as proc:
        assert proc.stdin is not None
        assert proc.stdout is not None
        assert proc.stderr is not None

        proc.stdin.write(cmd_stdin + "\n")
        proc.stdin.flush()
        proc.stdin.close()

        progress = None
        # Use select to read from both stdout and stderr
        streams = [proc.stdout, proc.stderr]
        while streams:
            readable, _, _ = select.select(streams, [], [], 0.1)

            for stream in readable:
                line = stream.readline()
                if not line:
                    # Stream closed
                    streams.remove(stream)
                    continue

                if stream == proc.stdout:
                    d = json.loads(line, object_hook=decode_clippy_json)
                    if _has_tqdm:
                        if progress is None:
                            if PROGRESS_START_KEY in d:
                                progress = (
                                    tqdm()
                                    if d[PROGRESS_START_KEY] is None
                                    else tqdm(total=d[PROGRESS_START_KEY])
                                )
                                # print(f"start, total = {d[PROGRESS_START_KEY]}, {progress.n=}")
                        else:
                            if PROGRESS_INC_KEY in d:
                                progress.update(d[PROGRESS_INC_KEY])
                                progress.refresh()
                                # print(f"update {progress.n=}")
                            if PROGRESS_SET_KEY in d:
                                progress.n = d[PROGRESS_SET_KEY]
                                progress.refresh()
                            if PROGRESS_END_KEY in d:
                                progress.close()
                                # print("close")
                                progress = None
                    if progress is None:
                        if OUTPUT_KEY in d:
                            print(d[OUTPUT_KEY])
                elif stream == proc.stderr:
                    stderr_lines.append(line)
                    print(line.rstrip(), flush=True)

            if proc.poll() is not None:
                # Process terminated, read any remaining output
                break

    print(f"{stderr_lines=}")
    print(f"{(stderr_lines==True)}")
    print(f"{(stderr_lines==False)}")
    stderr = "".join(stderr_lines) if stderr_lines else None
    print(f"{stderr=}")
    print(f"{(stderr is None)=}")
    print(f"{proc.returncode=}")
    if progress is not None:
        progress.close()
    if proc.returncode:
        print("RETURNCODE!!!!")
        raise (ClippyValidationError(stderr) if validate else ClippyBackendError(stderr))

    if not d:
        return None, stderr, proc.returncode
    if stderr:
        logger.debug('Received stderr: %s', stderr)
    if proc.returncode != 0:
        logger.debug("Process returned %d", proc.returncode)
    logger.debug('run(): final stdout = %s', d)

    return (d, stderr, proc.returncode)


def _validate(
    cmd: str | list[str], dct: AnyDict, logger: logging.Logger
) -> tuple[bool, str]:
    '''
    Converts the dictionary dct into a json file and calls executable cmd with the DRY_RUN_FLAG.
    Returns True/False (validation successful) and any stderr.
    '''

    if isinstance(cmd, str):
        cmd = [cmd]

    execcmd = cfg.get('validate_cmd_prefix').split() + cmd + [DRY_RUN_FLAG]
    logger.debug("Validating %s", cmd)

    _, stderr, retcode = _stream_exec(execcmd, dct, logger, validate=True)
    print(f"in validate {stderr=}")
    print(f"in validate {(stderr is not None)=}")
    print(f"in validate: {retcode=}")
    return retcode == 0, stderr or ""


def _run(cmd: str | list[str], dct: AnyDict, logger: logging.Logger) -> AnyDict:
    '''
    converts the dictionary dct into a json file and calls executable cmd. Prepends
    cmd_prefix configuration, if any.
    '''

    if isinstance(cmd, str):
        cmd = [cmd]
    execcmd = cfg.get('cmd_prefix').split() + cmd
    logger.debug('Running %s', execcmd)
    # should we do something with stderr?

    output, _, retcode = _stream_exec(execcmd, dct, logger, validate=False)
    if retcode != 0:
        logger.warning("Process returned non-zero return code: %d", retcde)
    return output or {}


def _help(cmd: str | list[str], dct: AnyDict, logger: logging.Logger) -> AnyDict:
    '''
    Retrieves the help output from the clippy command. Prepends validate_cmd_prefix
    if set and appends HELP_FLAG.
    Unlike `_validate()`, does not append DRY_RUN_FLAG, and returns the output.
    '''
    if isinstance(cmd, str):
        cmd = [cmd]
    execcmd = cfg.get('validate_cmd_prefix').split() + cmd + [HELP_FLAG]
    logger.debug('Running %s', execcmd)
    # should we do something with stderr?

    output, _, _ = _stream_exec(execcmd, dct, logger, validate=True)
    return output or {}
