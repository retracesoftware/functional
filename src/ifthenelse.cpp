#include "functional.h"
#include <structmember.h>

struct IfThenElse {
    PyObject_HEAD
    int from_arg;
    PyObject * test;
    vectorcallfunc test_vectorcall;

    PyObject * then;
    vectorcallfunc then_vectorcall;
    PyObject * otherwise;
    vectorcallfunc otherwise_vectorcall;
    vectorcallfunc vectorcall;
};

static PyObject * vectorcall(IfThenElse * self, PyObject** args, size_t nargsf, PyObject* kwnames) {

    PyObject * test_res = self->test_vectorcall(self->test, args + self->from_arg, PyVectorcall_NARGS(nargsf) - self->from_arg, kwnames);

    if (!test_res) return nullptr;
    int is_true = PyObject_IsTrue(test_res);
    Py_DECREF(test_res);

    switch (is_true) {
        case 0:
            return self->then_vectorcall(self->then, args, nargsf, kwnames);
        case 1:
            return self->otherwise_vectorcall(self->otherwise, args, nargsf, kwnames);
        default:
            return nullptr;
    }
}

static int traverse(IfThenElse* self, visitproc visit, void* arg) {
    Py_VISIT(self->test);
    Py_VISIT(self->then);
    Py_VISIT(self->otherwise);

    return 0;
}

static int clear(IfThenElse* self) {
    Py_CLEAR(self->test);
    Py_CLEAR(self->then);
    Py_CLEAR(self->otherwise);
    return 0;
}

static void dealloc(IfThenElse *self) {
    PyObject_GC_UnTrack(self);          // Untrack from the GC
    clear(self);
    Py_TYPE(self)->tp_free((PyObject *)self);  // Free the object
}

static int init(IfThenElse *self, PyObject *args, PyObject *kwds) {

    PyObject * test = NULL;
    PyObject * then = NULL;
    PyObject * otherwise = NULL;
    int from_arg = 0;

    static const char *kwlist[] = {
        "test",
        "then",
        "otherwise",
        "from_arg",
        NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OOO|I", 
        (char **)kwlist,
        &test,
        &then,
        &otherwise,
        &from_arg))
    {
        return -1; // Return NULL on failure
    }

    CHECK_CALLABLE(test);
    CHECK_CALLABLE(then);
    CHECK_CALLABLE(otherwise);
    
    self->test = Py_XNewRef(test);
    self->test_vectorcall = extract_vectorcall(test);
    self->then = Py_XNewRef(then);
    self->then_vectorcall = extract_vectorcall(then);
    self->otherwise = Py_XNewRef(otherwise);
    self->otherwise_vectorcall = extract_vectorcall(otherwise);
    self->vectorcall = (vectorcallfunc)vectorcall;
    self->from_arg = from_arg;

    return 0;
}

static PyMemberDef members[] = {
    // {"argument", T_OBJECT, OFFSET_OF_MEMBER(IfThenElse, argument), 0, "TODO"},
    // {"result", T_OBJECT, OFFSET_OF_MEMBER(IfThenElse, result), 0, "TODO"},
    // {"error", T_OBJECT, OFFSET_OF_MEMBER(IfThenElse, error), 0, "TODO"},
    // {"function", T_OBJECT, OFFSET_OF_MEMBER(IfThenElse, function), 0, "TODO"},
    {NULL}  /* Sentinel */
};

PyTypeObject IfThenElse_Type = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = MODULE "if_then_else",
    .tp_basicsize = sizeof(IfThenElse),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)dealloc,
    .tp_vectorcall_offset = OFFSET_OF_MEMBER(IfThenElse, vectorcall),
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
