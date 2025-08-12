#include "functional.h"
#include <structmember.h>

struct TransformCall : public PyObject {
    
    PyObject * function;
    PyObject * argument;
    PyObject * result;
    PyObject * error;

    vectorcallfunc vectorcall;
};

static PyObject * do_call(TransformCall * self, PyObject* const * args, size_t nargsf, PyObject* kwnames) {

    PyObject * result = PyObject_Vectorcall(self->function, args, nargsf, kwnames);

    if (result) {
        if (self->result) {

            PyObject * transformed = PyObject_CallOneArg(self->result, result);
            Py_DECREF(result);
            return transformed;
        }
    } else {
        if (self->error) {
            PyObject *ptype = NULL, *pvalue = NULL, *ptraceback = NULL;

            // Fetch the current exception
            PyErr_Fetch(&ptype, &pvalue, &ptraceback);

            result = PyObject_CallFunctionObjArgs(self->error,
                ptype ? ptype : Py_None,
                pvalue ? pvalue : Py_None,
                ptraceback ? ptraceback : Py_None,
                nullptr);

            Py_XDECREF(ptype);
            Py_XDECREF(pvalue);
            Py_XDECREF(ptraceback);

            // if (callback_result) {
            //     Py_DECREF(callback_result);
            //     PyErr_Restore(ptype, pvalue, ptraceback);
            // }
            // else {
            //     // new error was raised, delete these values
            //     Py_XDECREF(ptype);
            //     Py_XDECREF(pvalue);
            //     Py_XDECREF(ptraceback);
            // }
        }
    }
    return result;
}

static PyObject * vectorcall_new_from(
    TransformCall * self, 
    size_t from,
    PyObject * transformed,
    PyObject** args, 
    size_t nargs,
    PyObject* kwnames)
{
    PyObject * result = NULL;

    size_t all = nargs + (kwnames ? PyTuple_Size(kwnames) : 0);

    PyObject * on_stack[SMALL_ARGS + 1];
    PyObject ** buffer;

    if (all >= SMALL_ARGS) {
        buffer = (PyObject **)PyMem_Malloc(sizeof(PyObject *) * (all + 2));
        if (!buffer) {
            return NULL;
        }
    } else {
        buffer = on_stack;
    }

    for (size_t i = 0; i < from; i++) {
        buffer[i + 1] = Py_NewRef(args[i]);
    }

    buffer[from + 1] = transformed;

    for (size_t i = from + 1; i < all; i++) {

        PyObject * new_arg = PyObject_CallOneArg(self->argument, args[i]);

        if (!new_arg) {
            for (size_t j = i; j < all; j++) buffer[j + 1] = NULL;
            goto error;
        }
        buffer[i + 1] = new_arg;
    }
    result = do_call(self, buffer + 1, nargs | PY_VECTORCALL_ARGUMENTS_OFFSET, kwnames);

error:

    for (size_t i = 0; i < all; i++) {
        Py_XDECREF(buffer[i + 1]);
    }

    if (all >= SMALL_ARGS) {
        PyMem_Free(buffer);
    }
    return result;
}

static PyObject * vectorcall(TransformCall * self, PyObject** args, size_t nargsf, PyObject* kwnames) {

    size_t nargs = PyVectorcall_NARGS(nargsf) + (kwnames ? PyTuple_GET_SIZE(kwnames) : 0);
    
    if (self->argument) {
        for (size_t i = 0; i < nargs; i++) {

            PyObject * transformed = PyObject_CallOneArg(self->argument, args[i]);

            if (!transformed) {
                return NULL;
            }
            else if (transformed == args[i]) {
                Py_DECREF(transformed);
            } else {
                return vectorcall_new_from(self, i, transformed, args, PyVectorcall_NARGS(nargsf), kwnames);
            }
        }
    }
    return do_call(self, args, nargsf, kwnames);
}

static int traverse(TransformCall* self, visitproc visit, void* arg) {
    Py_VISIT(self->function);
    Py_VISIT(self->argument);
    Py_VISIT(self->result);
    Py_VISIT(self->error);

    return 0;
}

static int clear(TransformCall* self) {
    Py_CLEAR(self->function);
    Py_CLEAR(self->argument);
    Py_CLEAR(self->result);
    Py_CLEAR(self->error);
    return 0;
}

static void dealloc(TransformCall *self) {
    PyObject_GC_UnTrack(self);          // Untrack from the GC
    clear(self);
    Py_TYPE(self)->tp_free((PyObject *)self);  // Free the object
}

static int init(TransformCall *self, PyObject *args, PyObject *kwds) {

    PyObject * function = NULL;
    PyObject * argument = NULL;
    PyObject * result = NULL;
    PyObject * error = NULL;

    static const char *kwlist[] = {
        "function",
        "argument",
        "result",
        "error",
        NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|OOO", 
        (char **)kwlist,
        &function,
        &argument,
        &result,
        &error))
    {
        return -1; // Return NULL on failure
    }

    CHECK_CALLABLE(function);
    CHECK_CALLABLE(argument);
    CHECK_CALLABLE(result);
    CHECK_CALLABLE(error);
    
    self->function = Py_XNewRef(function);
    self->argument = Py_XNewRef(argument);
    self->result = Py_XNewRef(result);
    self->error = Py_XNewRef(error);
    self->vectorcall = (vectorcallfunc)vectorcall;

    return 0;
}

static PyMemberDef members[] = {
    {"argument", T_OBJECT, OFFSET_OF_MEMBER(TransformCall, argument), 0, "TODO"},
    {"result", T_OBJECT, OFFSET_OF_MEMBER(TransformCall, result), 0, "TODO"},
    {"error", T_OBJECT, OFFSET_OF_MEMBER(TransformCall, error), 0, "TODO"},
    {"function", T_OBJECT, OFFSET_OF_MEMBER(TransformCall, function), 0, "TODO"},
    {NULL}  /* Sentinel */
};

PyTypeObject TransformCall_Type = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = MODULE "transform",
    .tp_basicsize = sizeof(TransformCall),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)dealloc,
    .tp_vectorcall_offset = OFFSET_OF_MEMBER(TransformCall, vectorcall),
    .tp_call = PyVectorcall_Call,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_HAVE_VECTORCALL,
    .tp_doc = "TODO",
    .tp_traverse = (traverseproc)traverse,
    .tp_clear = (inquiry)clear,
    // .tp_methods = methods,
    .tp_members = members,
    .tp_init = (initproc)init,
    .tp_new = PyType_GenericNew,
};
