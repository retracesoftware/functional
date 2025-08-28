import retracesoftware_functional as _functional
from retracesoftware_functional import *

def when(test, then):
    return _functional.if_then_else(test, then, _functional.identity)

def when_instanceof(cls, then):
    return when(_functional.isinstanceof(cls), then)


