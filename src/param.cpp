#include "functional.h"
#include <structmember.h>

struct Param : public PyObject {
    int index;
    PyObject * name;
    vectorcallfunc vectorcall;

    static PyObject * repr(Param *self) {
        return PyUnicode_FromFormat(MODULE "param(name = %S index = %i)", self->name, self->index);
    }

    static PyObject * call(Param * self, PyObject * const * args, size_t nargsf, PyObject* kwnames) {

        int nargs = PyVectorcall_NARGS(nargsf);

        if (kwnames) {
            for (int i = 0; i < PyTuple_GET_SIZE(kwnames); i++) {
                 PyObject * item = PyTuple_GET_ITEM(kwnames, i);

                 if (item == self->name) {
                    return Py_NewRef(args[nargs + i]);
                 }
            }
        }
        
        if (nargs > self->index) {
            return Py_NewRef(args[self->index]);
        }
        else {
            PyErr_Format(PyExc_ValueError, "Parameter: %S wasn't passed on call", self->name);
            return nullptr;
        }
    }

    static int init(Param *self, PyObject *args, PyObject *kwds) {

        PyObject * name;
        int index;

        static const char *kwlist[] = {
            "name",
            "index",
            NULL};

        if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!i", 
            (char **)kwlist,
            &PyUnicode_Type,
            &name,
            &index))
        {
            return -1; // Return NULL on failure
        }
        
        self->name = Py_NewRef(name);
        self->index = index;
        self->vectorcall = (vectorcallfunc)call;
        return 0;
    }

    static void dealloc(Param *self) {
        Py_DECREF(self->name);
        Py_TYPE(self)->tp_free((PyObject *)self);  // Free the object
    }

};

static PyMemberDef members[] = {
    {"name", T_OBJECT, OFFSET_OF_MEMBER(Param, name), READONLY, "TODO"},
    {"index", T_INT, OFFSET_OF_MEMBER(Param, index), READONLY, "TODO"},
    {NULL}  /* Sentinel */
};

PyTypeObject Param_Type = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = MODULE "param",
    .tp_basicsize = sizeof(Param),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)Param::dealloc,
    .tp_vectorcall_offset = OFFSET_OF_MEMBER(Param, vectorcall),
    .tp_repr = (reprfunc)Param::repr,
    .tp_call = PyVectorcall_Call,
    .tp_str = (reprfunc)Param::repr,

    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_VECTORCALL,
    .tp_doc = "TODO",
    // .tp_traverse = (traverseproc)traverse,
    // .tp_clear = (inquiry)clear,
    // .tp_methods = methods,
    .tp_members = members,
    .tp_init = (initproc)Param::init,
    .tp_new = PyType_GenericNew,
};
