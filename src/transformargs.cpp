#include "functional.h"
#include <algorithm>
#include <structmember.h>
#include <signal.h>

struct TransformArgs {
    PyObject_HEAD
    int from;
    PyObject * func;
    PyObject * transform;
    vectorcallfunc vectorcall;
};

static PyObject * vectorcall(TransformArgs * self, PyObject* const * args, size_t nargsf, PyObject* kwnames) {

    size_t nargs = PyVectorcall_NARGS(nargsf);
    int all = nargs + (kwnames ? PyTuple_Size(kwnames) : 0);

    PyObject * on_stack[SMALL_ARGS + 1];
    PyObject ** buffer;
    PyObject * result = nullptr;

    if (all >= SMALL_ARGS) {
        buffer = (PyObject **)PyMem_Malloc(sizeof(PyObject *) * (all + 1));
        if (!buffer) {
            return nullptr;
        }
    } else {
        buffer = on_stack;
    }

    int from = std::min(self->from, (int)nargs);

    for (int i = 0; i < from; i++) {
        buffer[i + 1] = Py_NewRef(args[i]);
    }

    for (int i = from; i < all; i++) {
        buffer[i + 1] = PyObject_CallOneArg(self->transform, args[i]);
        if (!buffer[i + 1]) {
            for (int j = i; j < all; j++) {
                buffer[j + 1] = nullptr;
            }
            goto error;
        }
    }    
    result = PyObject_Vectorcall(self->func, 
                                 buffer + 1, 
                                 nargs | PY_VECTORCALL_ARGUMENTS_OFFSET,
                                 kwnames);
error:
    for (int i = 0; i < all; i++) {
        Py_XDECREF(buffer[i + 1]);
    }
    
    if (buffer != on_stack) {
        PyMem_Free(buffer);
    }

    return result;
}

static int traverse(TransformArgs* self, visitproc visit, void* arg) {
    Py_VISIT(self->transform);
    Py_VISIT(self->func);
    return 0;
}

static int clear(TransformArgs* self) {
    Py_CLEAR(self->transform);
    Py_CLEAR(self->func);
    return 0;
}

static void dealloc(TransformArgs *self) {
    PyObject_GC_UnTrack(self);          // Untrack from the GC
    clear(self);
    Py_TYPE(self)->tp_free((PyObject *)self);  // Free the object
}

static PyMemberDef members[] = {
    {nullptr}  /* Sentinel */
};

static PyObject * create(PyTypeObject *type, PyObject *args, PyObject *kwds) {

    PyObject * function;
    PyObject * transform;
    int from = 0;
    
    static const char *kwlist[] = {"function", "transform", "from_arg", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO|i", (char **)kwlist, &function, &transform, &from)) {
        return nullptr; // Return NULL on failure
    }
    
    TransformArgs * self = (TransformArgs *)type->tp_alloc(type, 0);

    if (!self) {
        return nullptr;
    }

    self->transform = Py_NewRef(transform);
    self->func = Py_NewRef(function);
    self->from = from;

    self->vectorcall = (vectorcallfunc)vectorcall;

    return (PyObject *)self;
}

PyTypeObject TransformArgs_Type = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = MODULE "TransformArgs",
    .tp_basicsize = sizeof(TransformArgs),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)dealloc,
    .tp_vectorcall_offset = offsetof(TransformArgs, vectorcall),
    .tp_call = PyVectorcall_Call,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_HAVE_VECTORCALL,
    .tp_doc = "TODO",
    .tp_traverse = (traverseproc)traverse,
    .tp_clear = (inquiry)clear,
    // .tp_methods = methods,
    .tp_members = members,
    .tp_new = (newfunc)create,
};
