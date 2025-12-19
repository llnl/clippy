# Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
# See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: MIT

"""This file contains custom Clippy errors."""


class ClippyError(Exception):
    """
    This is a top-level custom exception for Clippy.
    """


class ClippyConfigurationError(ClippyError):
    """
    This error represents a configuration error on user input.
    """


class ClippyBackendError(ClippyError):
    """
    This error should be thrown when the backend returns an abend.
    """


class ClippyValidationError(ClippyError):
    """
    This error represents a validation error in the inputs to a clippy job.
    """


class ClippySerializationError(ClippyError):
    """
    This error should be thrown when clippy object serialization fails
    """


class ClippyClassInconsistencyError(ClippyError):
    """
    This error represents a class inconsistency error (name or docstring mismatch).
    """


class ClippyTypeError(ClippyError):
    """
    This error represents an error with the type of data being passed to the back end.
    """


class ClippyInvalidSelectorError(ClippyError):
    """
    This error represents an error with a selector that is not defined for a given clippy class.
    """
