"""
Functions to check versions. Requires semver.
"""

from __future__ import annotations

from semver import Version

from .. import cfg
from ..clippy_types import AnyDict


def _check_version(output_dict: AnyDict | None) -> bool:
    """
    Given an output dictionary, match the version number against config.
    Return true if version is compatible with version range set in config,
    false otherwise. If no version is found, return false.
    """
    if output_dict is None:
        return False
    if "version" not in output_dict:
        return False
    backend_ver = Version.parse(output_dict["version"])
    config_vers = cfg.get("required_versions").split(",")
    return all(backend_ver.match(v.strip()) for v in config_vers)
