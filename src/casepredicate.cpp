#include "functional.h"
#include "object.h"
#include "pyerrors.h"
#include <structmember.h>

struct CasePredicate {
    PyObject_HEAD
    PyObject * elements;
    vectorcallfunc vectorcall;
};

static int run_predicate(PyObject * pred, PyObject** args, size_t nargsf, PyObject* kwnames) {
    PyObject * res = PyObject_Vectorcall(pred, args, nargsf, kwnames);

    if (!res) return -1;
    int status = PyObject_IsTrue(res);
    Py_DECREF(res);
    return status;
}

static PyObject * vectorcall(CasePredicate * self, PyObject** args, size_t nargsf, PyObject* kwnames) {

    size_t n = PyTuple_GET_SIZE(self->elements);

    for (size_t i = 0; i < (n & ~1); i += 2) {
        PyObject * pred = PyTuple_GET_ITEM(self->elements, i);

        switch (run_predicate(pred, args, nargsf, kwnames)) {
            case 0: break;
            case 1: return PyObject_Vectorcall(PyTuple_GET_ITEM(self->elements, i + 1), args, nargsf, kwnames);
            default:
                assert (PyErr_Occurred());
                return nullptr;
        }
    }
    if (n % 2 == 1) {
        PyObject * last = PyTuple_GET_ITEM(self->elements, n - 1);

        return PyObject_Vectorcall(last, args, nargsf, kwnames);
    }
    Py_RETURN_NONE;
}

static int traverse(CasePredicate* self, visitproc visit, void* arg) {
    Py_VISIT(self->elements);

    return 0;
}

static int clear(CasePredicate* self) {
    Py_CLEAR(self->elements);
    return 0;
}

static void dealloc(CasePredicate *self) {
    PyObject_GC_UnTrack(self);          // Untrack from the GC
    clear(self);
    Py_TYPE(self)->tp_free((PyObject *)self);  // Free the object
}

static PyMemberDef members[] = {
    {"elements", T_OBJECT, offsetof(CasePredicate, elements), READONLY, "TODO"},
    {NULL}  /* Sentinel */
};

static PyObject * create(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    
    CasePredicate * self = (CasePredicate *)type->tp_alloc(type, 0);

    if (!self) {
        return NULL;
    }

    self->elements = Py_NewRef(args);

    self->vectorcall = (vectorcallfunc)vectorcall;

    return (PyObject *)self;
}

PyTypeObject CasePredicate_Type = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = MODULE "case_predicate",
    .tp_basicsize = sizeof(CasePredicate),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)dealloc,
    .tp_vectorcall_offset = offsetof(CasePredicate, vectorcall),
    .tp_call = PyVectorcall_Call,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_HAVE_VECTORCALL,
    .tp_doc = "TODO",
    .tp_traverse = (traverseproc)traverse,
    .tp_clear = (inquiry)clear,
    // .tp_methods = methods,
    .tp_members = members,
    .tp_new = (newfunc)create,
};
