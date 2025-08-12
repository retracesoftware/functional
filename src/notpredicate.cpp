#include "functional.h"
#include <structmember.h>

struct NotPredicate {
    PyObject_HEAD
    PyObject * pred;
    vectorcallfunc vectorcall;
};

static PyObject * vectorcall(NotPredicate * self, PyObject** args, size_t nargsf, PyObject* kwnames) {

    PyObject * result = PyObject_Vectorcall(self->pred, args, nargsf, kwnames);

    if (!result) return NULL;

    switch (PyObject_IsTrue(result)) {
        case 0: 
            Py_DECREF(result);
            Py_RETURN_TRUE;
        case 1:
            Py_DECREF(result);
            Py_RETURN_FALSE;
        default:
            Py_DECREF(result);
            return NULL;
    }
}

static int traverse(NotPredicate* self, visitproc visit, void* arg) {
    Py_VISIT(self->pred);

    return 0;
}

static int clear(NotPredicate* self) {
    Py_CLEAR(self->pred);
    return 0;
}

static void dealloc(NotPredicate *self) {
    PyObject_GC_UnTrack(self);          // Untrack from the GC
    clear(self);
    Py_TYPE(self)->tp_free((PyObject *)self);  // Free the object
}

static PyMemberDef members[] = {
    {"pred", T_OBJECT, offsetof(NotPredicate, pred), READONLY, "TODO"},
    {NULL}  /* Sentinel */
};

static PyObject * create(PyTypeObject *type, PyObject *args, PyObject *kwds) {

    PyObject * pred;
    
    static const char *kwlist[] = {"pred", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", (char **)kwlist, &pred))
    {
        return NULL; // Return NULL on failure
    }
    
    NotPredicate * self = (NotPredicate *)type->tp_alloc(type, 0);

    if (!self) {
        return NULL;
    }

    self->pred = Py_NewRef(pred);

    self->vectorcall = (vectorcallfunc)vectorcall;

    return (PyObject *)self;
}

static PyObject * tp_str(NotPredicate * self) {
    return PyUnicode_FromFormat("%s(%S)", Py_TYPE(self)->tp_name, self->pred);
}

PyTypeObject NotPredicate_Type = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = MODULE "not_predicate",
    .tp_basicsize = sizeof(NotPredicate),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)dealloc,
    .tp_vectorcall_offset = offsetof(NotPredicate, vectorcall),
    .tp_call = PyVectorcall_Call,
    .tp_str = (reprfunc)tp_str,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_HAVE_VECTORCALL,
    .tp_doc = "TODO",
    .tp_traverse = (traverseproc)traverse,
    .tp_clear = (inquiry)clear,
    // .tp_methods = methods,
    .tp_members = members,
    .tp_new = (newfunc)create,
};
