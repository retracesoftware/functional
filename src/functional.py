import retracesoftware_functional as _functional
from retracesoftware_functional import *

def when_not(test, then):
    return _functional.if_then_else(test, None, then)

def when(test, then):
    return _functional.if_then_else(test, then, None)

def when_instanceof(cls, then):
    return when(_functional.isinstanceof(cls), then)

repeatedly = always

# TODO, compose is wrong order at the moment

def sequence(*args):
    if len(args) == 0:
        raise Exception()
    elif len(args) == 1:
        return args[0]
    elif len(args) == 2:        
        # sequence g, f
        # f(g(x))
        return compose(args[1], args[0])
    else:
        return compose(args[-1], sequence(*args[:-1]))
        # raise Exception('TODO - support sequence with more than 2 args')

def wrap(transform, f):
    return mapargs(transform = transform, function = sequence(f, transform))

def recursive_wrap(pred, transform, f):
    return wrap(when(pred, partial(wrap, transform)), f)
