"""
Utility functions
"""

from .clippy_types import AnyDict
from .constants import KEYPATH_KEY
from .error import ClippyInvalidKeypathError


def flat_dict_to_nested(input_dict: AnyDict) -> AnyDict:
    """input dictionary has dot-delineated keys which are then parsed as subkeys.
    That is: {'a.b.c': 5} becomes {'a': {'_keypaths': {'b': {'_keypaths': {'c': {'__doc__' = 5}}}}}}
    assuming `KEYPATH_KEY == '_keypaths'`
    """

    output_dict: AnyDict = {}
    for k, v in input_dict.items():
        # k is dotted
        if "." not in k:
            raise ClippyInvalidKeypathError("cannot set top-level keypaths")

        *path, last = k.split(".")
        if last.startswith("_"):
            raise ClippyInvalidKeypathError("keypaths must not start with an underscore.")
        curr_nest = output_dict
        for p in path:
            if p.startswith("_"):
                raise ClippyInvalidKeypathError("keypaths must not start with an underscore.")
            curr_nest.setdefault(p, {})
            curr_nest[p].setdefault(KEYPATH_KEY, {})
            curr_nest = curr_nest[p][KEYPATH_KEY]

        curr_nest.setdefault(last, {"__doc__": v})
    return output_dict
