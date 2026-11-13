"""Custom keypaths for clippy."""

from __future__ import annotations

import jsonlogic as jl

from . import constants
from .clippy_types import AnyDict


class Keypath(jl.Variable):
    """A Keypath represents a single variable."""

    def __init__(self, parent: Keypath | None, name: str, docstr: str):
        super().__init__(name, docstr)  # op and o2 are None to represent this as a variable.
        self._parent = parent
        self._name = name
        self._fullname: str = self._name if self._parent is None else f"{self._parent._fullname}.{self._name}"
        self._subpaths: set[Keypath] = set()

    def __hash__(self):
        return hash(self._fullname)

    def _prepare(self):
        return {"var": self._fullname}

    def _hierarchy(self, acc: list[tuple[str, str]] | None = None):
        if acc is None:
            acc = []
        acc.append((self._fullname, self.__doc__ or ""))
        for subpath in self._subpaths:
            subpath._hierarchy(acc)
        return acc

    def _describe(self):
        hier = self._hierarchy()
        maxlen = max(len(sub_desc[0]) for sub_desc in hier)
        return "\n".join(f"{sub_desc[0]:<{maxlen + 2}} {sub_desc[1]}" for sub_desc in hier)

    def __str__(self):
        return repr(self._prepare())

    def _add_subpath(self, name: str, docstr: str):
        """add a subpath to this keypath"""
        subpath = Keypath(self, name, docstr)
        setattr(self, name, subpath)
        self._subpaths.add(subpath)

    def _del_subpath(self, name: str):
        """remove a subpath from this keypath"""
        self._subpaths.remove(getattr(self, name))
        delattr(self, name)

    def _clear_subpaths(self):
        """removes all subpaths"""
        for subpath in self._subpaths:
            delattr(self, subpath._name)
        self._subpaths = set()

    def _import_from_dict(self, d: AnyDict, merge: bool = False):
        """Imports subpaths from a dictionary.
        If `merge = True`, do not clear subpaths first.
        """

        # clear all children
        if not merge:
            self._clear_subpaths()

        for name, subdict in d.get(constants.KEYPATH_KEY, {}).items():
            docstr = subdict.get("__doc__", "")
            self._add_subpath(name, docstr)
            getattr(self, name)._import_from_dict(subdict)
