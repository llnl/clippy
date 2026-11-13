"""Point clippy at the checked-in fake backend before it is imported."""

import os
import pathlib
import sys

_HERE = pathlib.Path(__file__).resolve().parent
_BACKEND = _HERE / "backend"
_SRC = _HERE.parent / "src"

os.environ["CLIPPY_BACKEND_PATH"] = os.pathsep.join(
    p for p in (str(_BACKEND), os.environ.get("CLIPPY_BACKEND_PATH", "")) if p
)
sys.path.insert(0, str(_SRC))
