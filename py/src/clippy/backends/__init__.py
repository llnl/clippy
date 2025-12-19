"""
This module holds backend interfaces to clippy.

All backends must expose a `classes()` method that
returns a dict of {name: class} to import into clippy and
a `get_cfg()` method that returns a CLIPPY_CONFIG instance.
"""
