# Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
# See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: MIT
"""
Defines types and classes for Clippy.
"""

import os
from typing import Any

from .error import ClippyConfigurationError

# AnyDict is a convenience type so we can find places
# to be more specific in the future.
AnyDict = dict[str, Any]

# CONFIG_ENTRY is a convenience type for use in CLIPPY_CONFIG.
CONFIG_ENTRY = tuple[str | None, Any]


# CLIPPY_CONFIG holds configuration items for both
# global settings as well as backend settings.
# All access to config should be via the `get()` method.
class CLIPPY_CONFIG:
    def __init__(self, d):
        self._entries = d

    def get(self, field) -> Any:
        if field not in self._entries:
            raise ClippyConfigurationError(f"unknown configuration setting {field}")
        env, val = self._entries[field]
        if env is None:
            return val
        return os.environ.get(env, val)
