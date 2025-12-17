# Constants unique to the monolith backend.
# the flag to pass for a dry run to make sure syntax is proper

# format is command and key in return dict
CLASSES = ("_getclasses", "_classes")

STATUS_KEY = "_status"
STATUS_READY = {STATUS_KEY: "ready"}
READY_TIMEOUT = 5.0  # seconds
STATUS_START = {STATUS_KEY: "start"}
STATUS_END = {STATUS_KEY: "end"}
STATUS_UPDATE = {STATUS_KEY: "update"}

SELECT_TIMEOUT = 2.0  # seconds
RESULTS_KEY = "results"
CLASS_METHOD_KEY = "methods"
CLASS_NAME_KEY = "name"
CLASS_DOCSTR = "doc"
