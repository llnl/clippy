# pylint: disable=consider-using-namedtuple-or-dataclass
import os

_monolith_config_entries = {
    # backend path for executables, in addition to the CLIPPY_BACKEND_PATH environment variable.
    # Add to it here.
    # TODO: support multiple executables, one per class?
    "monolith_exe": os.environ.get("CLIPPY_MONOLITH_EXE", "")
}
