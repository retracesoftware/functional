import retracesoftware_functional as _functional
from retracesoftware_functional import *

def when_not(test, then):
    return _functional.if_then_else(test, None, then)

def when(test, then):
    return _functional.if_then_else(test, then, None)

def when_instanceof(cls, then):
    return when(_functional.isinstanceof(cls), then)

# repeatedly = always

# TODO, compose is wrong order at the moment

def lazy(func, *args):
    return _functional.partial(func, *args, required = 0)

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

droparg = dropargs

def wrap(transform, f):
    return mapargs(transform = transform, function = sequence(f, transform))

def wrap_function(transform, f):
    return mapargs(transform = transform, function = f)
    return sequence(mapargs(transform, f), transform)

def recurive_wrap_function(transform, f):
    assert callable(f)

    print(f'in recurive_wrap_function, being passed: {transform} {f}')

    rec = partial(recurive_wrap_function, transform)

    # def rec(obj):
    #     if callable(obj):
    #         return recurive_wrap_function(transform, obj)
    #     else:
    #         return obj

    def then(obj):
        print(f'obj {obj} IS callable!!!!')
        res = transform(obj)
        print(f'transformed: {res}')
        return res
    
    def other(obj):
        print(f'obj {obj} was not callable!!!')
        return obj
    
    return wrap_function(if_then_else(callable, then, other), transform(f))
    # return wrap_function(when(callable, sequence(transform, rec)), transform(f))

def recursive_wrap(pred, transform, f):
    return wrap(when(pred, partial(wrap, transform)), f)

def first(x): return x[0]