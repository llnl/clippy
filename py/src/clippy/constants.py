"""Clippy constants."""

# These should probably not be changed, which is why they're not in config.

# The name of the selector key in the meta.json files.
# these selectors cannot be dropped.
INITIAL_SELECTOR_KEY = "initial_selectors"

######
# Keys found in output from backend.
######
# The name of the selector key in the json returns.
# If this key exists in the results
# then the selector hierarchy is replaced with its (processed) contents.
SELECTOR_KEY = "_selectors"
# Any internal state kept in the Python frontend.
STATE_KEY = "_state"
# This will reference a string to be printed to stdout by the frontend.
OUTPUT_KEY = "stdout"
# This key is a boolean - if true, return self (to enable method chaining).
SELF_KEY = "returns_self"
# key to json entry that holds reference overrides from backend functions.
REFERENCE_KEY = "references"
# key to json entry that holds return data from backend functions.
RETURN_KEY = "returns"


# these keys are returned from the _help output when creating methods
# for classes.
METHODNAME_KEY = "method_name"
DOCSTRING_KEY = "desc"
ARGS_KEY = "args"


REAL = "real"
STRING = "string"
UINT = "uint"
INT = "int"
CLASS_META_FILE = "meta.json"
