"""
Utility functions
"""

from .clippy_types import AnyDict
from .constants import SELECTOR_KEY
from .error import ClippyInvalidSelectorError


def flat_dict_to_nested(input_dict: AnyDict) -> AnyDict:
    """input dictionary has dot-delineated keys which are then parsed as subkeys.
    That is: {'a.b.c': 5} becomes {'a': {'_selectors': {'b': {'_selectors': {'c': {'__doc__' = 5}}}}}}
    assuming `SELECTOR_KEY == '_selectors'`
    """

    output_dict: AnyDict = {}
    for k, v in input_dict.items():
        # k is dotted
        if "." not in k:
            raise ClippyInvalidSelectorError("cannot set top-level selectors")

        *path, last = k.split(".")
        if last.startswith("_"):
            raise ClippyInvalidSelectorError("selectors must not start with an underscore.")
        curr_nest = output_dict
        for p in path:
            if p.startswith("_"):
                raise ClippyInvalidSelectorError("selectors must not start with an underscore.")
            curr_nest.setdefault(p, {})
            curr_nest[p].setdefault(SELECTOR_KEY, {})
            curr_nest = curr_nest[p][SELECTOR_KEY]

        curr_nest.setdefault(last, {"__doc__": v})
    return output_dict
