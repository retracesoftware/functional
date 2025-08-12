#include "functional.h"
#include <structmember.h>

struct Repeatedly {
    PyObject_HEAD
    PyObject * f;
    vectorcallfunc vectorcall;
};

static PyObject * vectorcall(Repeatedly * self, PyObject** args, size_t nargsf, PyObject* kwnames) {
    return PyObject_CallNoArgs(self->f);
}

static int traverse(Repeatedly* self, visitproc visit, void* arg) {
    Py_VISIT(self->f);
    return 0;
}

static int clear(Repeatedly* self) {
    Py_CLEAR(self->f);
    return 0;
}

static void dealloc(Repeatedly *self) {
    PyObject_GC_UnTrack(self);          // Untrack from the GC
    clear(self);
    Py_TYPE(self)->tp_free((PyObject *)self);  // Free the object
}

static PyMemberDef members[] = {
    {"function", T_OBJECT, offsetof(Repeatedly, f), READONLY, "TODO"},
    {NULL}  /* Sentinel */
};

static PyObject * create(PyTypeObject *type, PyObject *args, PyObject *kwds) {

    PyObject * function;
    
    static const char *kwlist[] = {"function", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", (char **)kwlist, &function))
    {
        return NULL; // Return NULL on failure
    }
    
    Repeatedly * self = (Repeatedly *)type->tp_alloc(type, 0);

    if (!self) {
        return NULL;
    }

    self->f = Py_NewRef(function);

    self->vectorcall = (vectorcallfunc)vectorcall;

    return (PyObject *)self;
}

PyTypeObject Repeatedly_Type = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = MODULE "repeatedly",
    .tp_basicsize = sizeof(Repeatedly),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)dealloc,
    .tp_vectorcall_offset = offsetof(Repeatedly, vectorcall),
    .tp_call = PyVectorcall_Call,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_HAVE_VECTORCALL,
    .tp_doc = "TODO",
    .tp_traverse = (traverseproc)traverse,
    .tp_clear = (inquiry)clear,
    // .tp_methods = methods,
    .tp_members = members,
    .tp_new = (newfunc)create,
};
