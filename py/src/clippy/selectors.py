"""Custom selectors for clippy."""

from __future__ import annotations

import jsonlogic as jl

from . import constants
from .clippy_types import AnyDict

class Selector(jl.Variable):
    """A Selector represents a single variable."""

    def __init__(self, parent: Selector | None, name: str, docstr: str):
        super().__init__(name, docstr)  # op and o2 are None to represent this as a variable.
        self._parent = parent
        self._name = name
        self._fullname: str = self._name if self._parent is None else f"{self._parent._fullname}.{self._name}"
        self._subselectors: set[Selector] = set()

    def __hash__(self):
        return hash(self._fullname)

    def _prepare(self):
        return {"var": self._fullname}

    def _hierarchy(self, acc: list[tuple[str, str]] | None = None):
        if acc is None:
            acc = []
        acc.append((self._fullname, self.__doc__ or ""))
        for subsel in self._subselectors:
            subsel._hierarchy(acc)
        return acc

    def _describe(self):
        hier = self._hierarchy()
        maxlen = max(len(sub_desc[0]) for sub_desc in hier)
        return "\n".join(f"{sub_desc[0]:<{maxlen + 2}} {sub_desc[1]}" for sub_desc in hier)

    def __str__(self):
        return repr(self._prepare())

    def _to_serial(self):
        return {"var": self._fullname}

    def _add_subselector(self, name: str, docstr: str):
        """add a subselector to this selector"""
        subsel = Selector(self, name, docstr)
        setattr(self, name, subsel)
        self._subselectors.add(subsel)

    def _del_subselector(self, name: str):
        delattr(self, name)
        self._subselectors.remove(getattr(self, name))

    def _clear_subselectors(self):
        """removes all subselectors"""
        for subsel in self._subselectors:
            delattr(self, subsel._name)
        self._subselectors = set()

    def _import_from_dict(self, d: AnyDict, merge: bool = False):
        """Imports subselectors from a dictionary.
        If `merge = True`, do not clear subselectors first.
        """

        # clear all children
        if not merge:
            self._clear_subselectors()

        for name, subdict in d.get(constants.SELECTOR_KEY, {}).items():
            docstr = subdict.get("__doc__", "")
            self._add_subselector(name, docstr)
            getattr(self, name)._import_from_dict(subdict)
