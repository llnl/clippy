"""
Functions to execute backend programs.
"""

from __future__ import annotations

import contextlib
import json
import logging
import os
import select
import subprocess

from ... import cfg
from ...clippy_types import AnyDict
from ..serialization import decode_clippy_json, encode_clippy_json
from .constants import DRY_RUN_FLAG, HELP_FLAG


class NonZeroReturnCodeError(Exception):
    def __init__(self, execcmd, return_code, extramsg, message="returned non-zero exit code"):
        self.return_code = return_code
        self.execcmd = execcmd
        super().__init__(f"{execcmd} {message}: {return_code}\n{extramsg}")

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

    logger.debug(f"Submission = {submission_dict}")
    # PP support passing objects
    # ~ cmd_stdin = json.dumps(submission_dict)
    cmd_stdin = json.dumps(submission_dict, default=encode_clippy_json)

    logger.debug("Calling %s with input %s", cmd, cmd_stdin)

    d = {}
    stderr_lines = []

    with subprocess.Popen(
        cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, encoding="utf8"
    ) as proc:
        assert proc.stdin is not None
        assert proc.stdout is not None
        assert proc.stderr is not None

        proc.stdin.write(cmd_stdin + "\n")
        proc.stdin.flush()
        proc.stdin.close()

        progress = None
        # Use select with file descriptors and non-blocking reads
        stdout_fd = proc.stdout.fileno()
        stderr_fd = proc.stderr.fileno()
        fds = [stdout_fd, stderr_fd]

        # Set non-blocking mode
        os.set_blocking(stdout_fd, False)
        os.set_blocking(stderr_fd, False)

        # Line buffers for partial reads
        stdout_buffer = ""
        stderr_buffer = ""

        while fds or proc.poll() is None:
            # Only select if we have fds to monitor
            if fds:
                readable, _, _ = select.select(fds, [], [], 0.1)
            else:
                readable = []

            for fd in readable:
                try:
                    chunk = os.read(fd, 4096)
                    if not chunk:
                        # Stream closed (EOF)
                        fds.remove(fd)
                        continue

                    text = chunk.decode("utf-8")

                    if fd == stdout_fd:
                        stdout_buffer += text
                        while "\n" in stdout_buffer:
                            line, stdout_buffer = stdout_buffer.split("\n", 1)
                            d = json.loads(line, object_hook=decode_clippy_json)
                    elif fd == stderr_fd:
                        stderr_buffer += text
                        while "\n" in stderr_buffer:
                            line, stderr_buffer = stderr_buffer.split("\n", 1)
                            stderr_lines.append(line + "\n")
                            print(line, flush=True)
                except BlockingIOError:
                    # No data available right now
                    continue
                except OSError:
                    # Stream closed
                    if fd in fds:
                        fds.remove(fd)
                    continue

        # Process any remaining buffered data
        if stdout_buffer.strip():
            with contextlib.suppress(json.JSONDecodeError):
                d = json.loads(stdout_buffer, object_hook=decode_clippy_json)

        if stderr_buffer.strip():
            stderr_lines.append(stderr_buffer)
            print(stderr_buffer.rstrip(), flush=True)

    stderr = "".join(stderr_lines) if stderr_lines else None
    if progress is not None:
        progress.close()
    # if proc.returncode:
    #     raise (ClippyValidationError(stderr) if validate else ClippyBackendError(stderr))

    if not d:
        return None, stderr, proc.returncode
    if stderr:
        logger.debug("Received stderr: %s", stderr)
    if proc.returncode != 0:
        logger.debug("Process returned %d", proc.returncode)
    logger.debug("run(): final stdout = %s", d)

    return (d, stderr, proc.returncode)


def _validate(cmd: str | list[str], dct: AnyDict, logger: logging.Logger) -> tuple[bool, str]:
    """
    Converts the dictionary dct into a json file and calls executable cmd with the DRY_RUN_FLAG.
    Returns True/False (validation successful) and any stderr.
    """

    if isinstance(cmd, str):
        cmd = [cmd]

    execcmd = cfg.get("validate_cmd_prefix").split() + cmd + [DRY_RUN_FLAG]
    logger.debug("Validating %s", cmd)

    _, stderr, retcode = _stream_exec(execcmd, dct, logger, validate=True)
    return retcode == 0, stderr or ""


def _run(cmd: str | list[str], dct: AnyDict, logger: logging.Logger) -> AnyDict:
    """
    converts the dictionary dct into a json file and calls executable cmd. Prepends
    cmd_prefix configuration, if any.
    """

    if isinstance(cmd, str):
        cmd = [cmd]
    execcmd = cfg.get("cmd_prefix").split() + cmd
    logger.debug("Running %s", execcmd)
    # should we do something with stderr?

    output, stderr, retcode = _stream_exec(execcmd, dct, logger, validate=False)
    if retcode != 0:
        raise NonZeroReturnCodeError(execcmd, retcode, stderr)
    return output or {}


def _help(cmd: str | list[str], dct: AnyDict, logger: logging.Logger) -> AnyDict:
    """
    Retrieves the help output from the clippy command. Prepends validate_cmd_prefix
    if set and appends HELP_FLAG.
    Unlike `_validate()`, does not append DRY_RUN_FLAG, and returns the output.
    """
    if isinstance(cmd, str):
        cmd = [cmd]
    execcmd = cfg.get("validate_cmd_prefix").split() + cmd + [HELP_FLAG]
    logger.debug("Running %s", execcmd)
    # should we do something with stderr?

    output, _, _ = _stream_exec(execcmd, dct, logger, validate=True)
    return output or {}
