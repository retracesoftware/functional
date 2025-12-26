#include "functional.h"
#include <structmember.h>

struct TypePredicate {
    PyObject_HEAD
    PyTypeObject * cls;
    vectorcallfunc vectorcall;
};

static PyObject * vectorcall(TypePredicate * self, PyObject** args, size_t nargsf, PyObject* kwnames) {
    return PyBool_FromLong(Py_TYPE(args[0]) == self->cls);
}

static int traverse(TypePredicate* self, visitproc visit, void* arg) {
    Py_VISIT(self->cls);
    return 0;
}

static int clear(TypePredicate* self) {
    Py_CLEAR(self->cls);
    return 0;
}

static void dealloc(TypePredicate *self) {
    PyObject_GC_UnTrack(self);          // Untrack from the GC
    clear(self);
    Py_TYPE(self)->tp_free((PyObject *)self);  // Free the object
}

static PyMemberDef members[] = {
    {"type", T_OBJECT, offsetof(TypePredicate, cls), READONLY, "TODO"},
    {NULL}  /* Sentinel */
};

static PyObject * create(PyTypeObject *type, PyObject *args, PyObject *kwds) {

    PyTypeObject * cls;
    
    static const char *kwlist[] = {"type", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!", (char **)kwlist, &PyType_Type, &cls))
    {
        return NULL; // Return NULL on failure
    }
    
    TypePredicate * self = (TypePredicate *)type->tp_alloc(type, 0);

    if (!self) {
        return NULL;
    }

    Py_INCREF(cls);
    self->cls = cls;

    self->vectorcall = (vectorcallfunc)vectorcall;

    return (PyObject *)self;
}

PyTypeObject TypePredicate_Type = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = MODULE "TypePredicate",
    .tp_basicsize = sizeof(TypePredicate),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)dealloc,
    .tp_vectorcall_offset = offsetof(TypePredicate, vectorcall),
    .tp_call = PyVectorcall_Call,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_HAVE_VECTORCALL,
    .tp_doc = "TODO",
    .tp_traverse = (traverseproc)traverse,
    .tp_clear = (inquiry)clear,
    // .tp_methods = methods,
    .tp_members = members,
    .tp_new = (newfunc)create,
};
