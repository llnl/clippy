"""
Clippy serialization functions and classes.
"""

from __future__ import annotations

from typing import Any

import jsonlogic as jl

from .. import _dynamic_types
from ..clippy_types import AnyDict
from ..error import ClippySerializationError

# TODO: SAB 20240204 complete typing here.


class ClippySerializable:
    """
    Declares the interface for serializing clippy objects. Subclass should inherit from this class to
    be serializable as part of the clippy frontend/backend communication infrustructure.

    The Clippy-type specification is as follows
    ```json
    {
        "__clippy_type__": {
            "__module__": <frontend-module>,
            "__class__": <clippy-type-name>,
            "state": {
                <state-attribute>: <state-attribute-value>
            }
        }
    }
    ```

    """

    def __init__(self):
        self._state = {}

    def to_serial(self) -> AnyDict:
        """
        Subclasses should override this method to provide a custom serialization of the object instance.
        This method should return a python-primitive (dict, list, str, etc) version of the instance state
        """
        return {
            "__clippy_type__": {
                "__module__": self.__class__.__module__,
                "__class__": self.__class__.__name__,
                "state": self._state,
            }
        }

    @classmethod
    def from_serial(cls, o: AnyDict):
        """
        Subclasses should override this method to provide a custom deserialization from the python-primative data.
        This method should return a fully constructed object instance.
        """

        if "__clippy_type__" not in o:
            raise ClippySerializationError("No clippy type detected")

        clippy_type = o["__clippy_type__"]
        assert isinstance(clippy_type, dict)
        # right now we aren't using the module but  probably should
        # type_module = clippy_type.get("__module__", None)
        type_name = clippy_type.get("__class__", None)
        state_dict = clippy_type.get("state", None)

        if type_name is None:
            raise ClippySerializationError("__clippy_type__.__class__ is unspecified")

        if type_name not in _dynamic_types:
            raise ClippySerializationError(f'"{type_name}" is not a known type, please clippy import it.')

        # get the type to deserialize into from the _dynamic_types dict
        # this does not account for the module the type may exist in
        t = _dynamic_types[type_name]

        if not issubclass(t, ClippySerializable):
            raise ClippySerializationError(f'"{type_name}" is not serializable.')

        # create an instance of the clippy type but avoid initializing it
        # because it may have required args we don't care about.
        # However we do want to call the ClippySerializable initializer
        # to give the instance the `state` attribute (and whatever else).
        instance = t.__new__(t)
        super(t, instance).__init__()

        # update the new instances state
        if state_dict is not None:
            instance._state.update(state_dict)
        return instance


def _form_method_arguments(method_args: AnyDict | None) -> tuple[list[Any], AnyDict]:
    if method_args is None:
        return [], {}

    sorted_method_args = sorted(
        [
            (
                method_args[arg_name]["position"],
                arg_name,
                method_args[arg_name]["arg_value"],
            )
            for arg_name in method_args
        ]
    )
    keyword_args = {arg[1]: arg[2] for arg in sorted_method_args if arg[0] == -1}
    positionals = [arg[2] for arg in sorted_method_args if arg[0] > -1]
    return (positionals, keyword_args)


def encode_clippy_json(o: Any) -> Any:
    """
    json encoder that is clippy-object aware.
    """
    if isinstance(o, jl.Operand):  # expression or variable
        return {"expression_type": "jsonlogic", "rule": o.prepare()}

    return o


def decode_clippy_json(o: Any) -> Any:
    """
    json decoder that is clippy-object aware.
    """
    if "__clippy_type__" in o:
        return ClippySerializable.from_serial(o)

    return o
