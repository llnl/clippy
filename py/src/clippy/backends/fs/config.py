# pylint: disable=consider-using-namedtuple-or-dataclass
import os

_fs_config_entries = {
    # backend path for executables, in addition to the CLIPPY_BACKEND_PATH environment variable. Add to it here.
    "fs_backend_paths": (
        None,
        [path for entry in os.environ.get("CLIPPY_BACKEND_PATH", "").split(":") if (path := entry.strip())],
    ),
    # exclude these directories from being made as classes.
    "fs_exclude_paths": ("CLIPPY_FS_EXCLUDE_PATHS", ["CMakeFiles"]),
}
