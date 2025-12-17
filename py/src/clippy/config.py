# pylint: disable=consider-using-namedtuple-or-dataclass
"""This holds a dictionary containing global configuration variables for clippy."""

# The format is config_key: (environment variable or None, default value)

import logging

from .clippy_types import CONFIG_ENTRY

_clippy_cfg: dict[str, CONFIG_ENTRY] = {
    # backends to use for Clippy.
    "backends": ("CLIPPY_BACKENDS", ["fs"]),
    # semver version restrictions for the backend
    "required_versions": ("CLIPPY_REQ_VERSIONS", ">=0.2.0"),
    # command prefix used to specify clippy task management with the HPC cluster
    # for instance, if using slurm this could be set to 'srun -n1 -ppdebug'
    "cmd_prefix": ("CLIPPY_CMD_PREFIX", ""),
    # command prefix used to specify clippy task management with the HPC cluster
    # for dry runs in certain environments. For instance, if using slurm this
    # could be set to 'srun -n1 -ppdebug'
    "validate_cmd_prefix": ("CLIPPY_VALIDATE_CMD_PREFIX", ""),
    # contol the log level of clippy
    "loglevel": ("CLIPPY_LOGLEVEL", logging.WARNING),
    "logformat": (
        "CLIPPY_LOGFORMAT",
        "%(asctime)s [%(filename)s:%(lineno)d (%(funcName)s) %(levelname)s: %(message)s",
    ),
    "logname": ("CLIPPY_LOGNAME", __name__),
}
