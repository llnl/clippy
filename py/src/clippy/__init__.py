"""This is the clippy initialization file."""

# The general flow is as follows:
# Create the configurations (see comments in .config for details)
# Create the logger
from __future__ import annotations

import importlib
import logging

from .clippy_types import CLIPPY_CONFIG, AnyDict
from .config import _clippy_cfg

# Create the main configuraton object and expose it globally.
cfg = CLIPPY_CONFIG(_clippy_cfg)

# expose for serialization.
_dynamic_types: AnyDict = {}

# Set up logging
logfmt = logging.Formatter(cfg.get("logformat"))
logger = logging.getLogger(cfg.get("logname"))
handler = logging.StreamHandler()
handler.setFormatter(logfmt)
logger.addHandler(handler)
logger.setLevel(cfg.get("loglevel"))


def load_classes():
    """For each listed backend, import the module of the same name. The
    backend should expose two functions: a classes() function that returns
    a dictionary of classes keyed by name, and a get_cfg() function that
    returns a CLIPPY_CONFIG object with backend-specific configuration.
    This object is then made an attribute of the global configuration
    (i.e., `cfg.fs.get('fs_specific_config')`).
    """
    for backend in cfg.get("backends"):
        b = importlib.import_module(f".backends.{backend}", package=__name__)
        setattr(cfg, backend, b.get_cfg())
        for name, c in b.classes().items():
            # backend_config = importlib.import_module(f".backends.{name}.config.{name}_config")
            # for k, v in backend_config.items():
            #     cfg._set(k, v)

            globals()[name] = c


load_classes()
