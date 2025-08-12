#include "functional.h"

struct Partial : public PyVarObject {
    vectorcallfunc vectorcall;
    // std::vector<std::pair<PyTypeObject *, PyObject *>> dispatch;
    PyObject * function;        
    PyObject * args[];

    static int clear(Partial* self) {
        Py_CLEAR(self->function);
        for (int i = 0; i < self->ob_size; i++) {
            Py_CLEAR(self->args[i]);
        }
        return 0;
    }
    
    static int traverse(Partial* self, visitproc visit, void* arg) {
        Py_VISIT(self->function);
        for (int i = 0; i < self->ob_size; i++) {
            Py_VISIT(self->args[i]);
        }
        return 0;
    }
    
    static void dealloc(Partial *self) {
        PyObject_GC_UnTrack(self);          // Untrack from the GC
        clear(self);
        Py_TYPE(self)->tp_free((PyObject *)self);  // Free the object
    }

    static PyObject * call(Partial * self, PyObject** args, size_t nargsf, PyObject* kwnames) {

        size_t nargs = PyVectorcall_NARGS(nargsf) + (kwnames ? PyTuple_GET_SIZE(kwnames) : 0);

        if (nargs == 0) {
            return PyObject_Vectorcall(self->function, self->args, self->ob_size, nullptr);
        } else {
            size_t total_args = self->ob_size + nargs;

            PyObject * on_stack[SMALL_ARGS + 1];
            PyObject ** buffer;

            if (total_args >= SMALL_ARGS) {
                buffer = (PyObject **)PyMem_Malloc(sizeof(PyObject *) * (total_args + 1));
                if (!buffer) {
                    return nullptr;
                }
            } else {
                buffer = on_stack;
            }

            for (size_t i = 0; i < (size_t)self->ob_size; i++) {
                buffer[i + 1] = self->args[i];
            }

            for (size_t i = 0; i < nargs; i++) {
                buffer[i + 1 + self->ob_size] = args[i];
            }

            nargsf = (self->ob_size + PyVectorcall_NARGS(nargsf)) | PY_VECTORCALL_ARGUMENTS_OFFSET;

            PyObject * result = PyObject_Vectorcall(self->function, buffer + 1, nargsf, kwnames);

            if (buffer != on_stack) {
                PyMem_Free(buffer);
            }
            return result;
        }
    }    
};

PyTypeObject Partial_Type = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = MODULE "partial",
    .tp_basicsize = sizeof(Partial),
    .tp_itemsize = sizeof(PyObject *),
    .tp_dealloc = (destructor)Partial::dealloc,
    .tp_vectorcall_offset = OFFSET_OF_MEMBER(Partial, vectorcall),
    .tp_call = PyVectorcall_Call,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_HAVE_VECTORCALL,
    .tp_doc = "TODO",
    .tp_traverse = (traverseproc)Partial::traverse,
    .tp_clear = (inquiry)Partial::clear,
    // .tp_methods = methods,
    // .tp_members = members,
    // .tp_new = (newfunc)create,
    // .tp_init = (initproc)TypePredWalker::init,
    // .tp_new = PyType_GenericNew,
};

PyObject * partial(PyObject * function, PyObject * const * args, size_t nargs) {

    Partial * self = (Partial *)Partial_Type.tp_alloc(&Partial_Type, nargs);
    
    if (!self) {
        return NULL;
    }

    self->ob_size = nargs;

    for (size_t i = 0; i < nargs; i++) {
        self->args[i] = Py_NewRef(args[i]);
    }
    self->vectorcall = (vectorcallfunc)Partial::call;
    self->function = Py_NewRef(function);

    return (PyObject *)self;
}

