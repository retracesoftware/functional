#include "functional.h"
#include "object.h"
#include "pyerrors.h"
#include <structmember.h>

struct Pair {
    vectorcallfunc vectorcall;
    PyObject * callable;
};

struct FirstOf : public PyVarObject {
    vectorcallfunc vectorcall;
    // std::vector<std::pair<PyTypeObject *, PyObject *>> dispatch;
    Pair dispatch[];
};

static PyObject * vectorcall(FirstOf * self, PyObject** args, size_t nargsf, PyObject* kwnames) {

    for (size_t i = 0; i < (size_t)self->ob_size; i++) {
        Pair * pair = self->dispatch + i;

        PyObject * res = pair->vectorcall(pair->callable, args, nargsf, kwnames);
        if (!res) return nullptr;
        else if (res == Py_None) {
            Py_DECREF(res);
        } else {
            return res;
        }
    }
    Py_RETURN_NONE;
}

static int traverse(FirstOf* self, visitproc visit, void* arg) {
    for (size_t i = 0; i < (size_t)self->ob_size; i++) {
        Py_VISIT(self->dispatch[i].callable);
    } 
    return 0;
}

static int clear(FirstOf* self) {
    for (size_t i = 0; i < (size_t)self->ob_size; i++) {
        Py_CLEAR(self->dispatch[i].callable);
    } 
    return 0;
}

static void dealloc(FirstOf *self) {
    PyObject_GC_UnTrack(self);          // Untrack from the GC
    clear(self);
    Py_TYPE(self)->tp_free((PyObject *)self);  // Free the object
}

static PyMemberDef members[] = {
    // {"elements", T_OBJECT, offsetof(CasePredicate, elements), READONLY, "TODO"},
    {NULL}  /* Sentinel */
};

PyTypeObject FirstOf_Type = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = MODULE "firstof",
    .tp_basicsize = sizeof(FirstOf),
    .tp_itemsize = sizeof(Pair),
    .tp_dealloc = (destructor)dealloc,
    .tp_vectorcall_offset = OFFSET_OF_MEMBER(FirstOf, vectorcall),
    .tp_call = PyVectorcall_Call,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_HAVE_VECTORCALL,
    .tp_doc = "TODO",
    .tp_traverse = (traverseproc)traverse,
    .tp_clear = (inquiry)clear,
    // .tp_methods = methods,
    .tp_members = members,
};

PyObject * firstof(PyObject * const * args, size_t nargs) {

    FirstOf * self = (FirstOf *)FirstOf_Type.tp_alloc(&FirstOf_Type, (nargs >> 1));
    
    if (!self) {
        return NULL;
    }

    for (int i = 0; i < nargs; i++) {
        self->dispatch[i].vectorcall = extract_vectorcall(args[i]);
        self->dispatch[i].callable = Py_NewRef(args[i]);
    }
    self->vectorcall = (vectorcallfunc)vectorcall;

    return (PyObject *)self;
}
