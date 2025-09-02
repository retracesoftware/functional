#include "functional.h"

struct Partial : public PyVarObject {
    vectorcallfunc vectorcall;
    // std::vector<std::pair<PyTypeObject *, PyObject *>> dispatch;
    PyObject *dict;
    PyObject * function;        
    vectorcallfunc function_vectorcall;
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

    static PyObject * getattro(Partial *self, PyObject *name) {
        return PyObject_GetAttr(self->function, name);
    }

    static PyObject * call(Partial * self, PyObject** args, size_t nargsf, PyObject* kwnames) {

        size_t nargs = PyVectorcall_NARGS(nargsf) + (kwnames ? PyTuple_GET_SIZE(kwnames) : 0);

        if (nargs == 0) {
            return self->function_vectorcall(self->function, self->args, self->ob_size, nullptr);
        } else {
            size_t total_args = self->ob_size + nargs;

            PyObject ** mem = (PyObject **)alloca(sizeof(PyObject *) * (total_args + 1)) + 1;

            for (size_t i = 0; i < (size_t)self->ob_size; i++) {
                mem[i] = self->args[i];
            }

            for (size_t i = 0; i < nargs; i++) {
                mem[i + self->ob_size] = args[i];
            }

            nargsf = (self->ob_size + PyVectorcall_NARGS(nargsf)) | PY_VECTORCALL_ARGUMENTS_OFFSET;

            return self->function_vectorcall(self->function, mem, nargsf, kwnames);
        }
    }

    static PyObject* create(PyTypeObject* type, PyObject* args, PyObject* kwds) {
        if (PyTuple_Size(args) == 0) {
            PyErr_SetString(PyExc_TypeError, "partial requires at least one positional argument");
            return nullptr;
        }

        // Use PyObject_NewVar to allocate memory for the object
        // Partial* self = (Partial *)Partial_Type.tp_alloc(&Partial_Type, PyTuple_Size(args) - 1);
        Partial* self = (Partial *)Partial_Type.tp_alloc(type, PyTuple_Size(args) - 1);
        
        // Check if the allocation was successful
        if (self == NULL) {
            return NULL; // Return NULL on error
        }

        self->function = Py_NewRef(PyTuple_GetItem(args, 0));
        self->function_vectorcall = extract_vectorcall(self->function);

        for (Py_ssize_t i = 0; i < self->ob_size; i++) {
            self->args[i] = Py_NewRef(PyTuple_GetItem(args, i + 1));
        }

        self->vectorcall = (vectorcallfunc)Partial::call;
        self->dict = NULL;

        return (PyObject*)self;
    }

    static PyObject* descr_get(PyObject *self, PyObject *obj, PyObject *type) {
        return obj == NULL || obj == Py_None ? Py_NewRef(self) : PyMethod_New(self, obj);
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
    .tp_getattro = (getattrofunc)Partial::getattro,
    .tp_flags = Py_TPFLAGS_DEFAULT | 
                Py_TPFLAGS_HAVE_GC | 
                Py_TPFLAGS_HAVE_VECTORCALL | 
                Py_TPFLAGS_METHOD_DESCRIPTOR |
                Py_TPFLAGS_BASETYPE,
    .tp_doc = "TODO",
    .tp_traverse = (traverseproc)Partial::traverse,
    .tp_clear = (inquiry)Partial::clear,
    .tp_descr_get = Partial::descr_get,
    .tp_dictoffset = OFFSET_OF_MEMBER(Partial, dict), // Set the offset here

    // .tp_methods = methods,
    // .tp_members = members,
    .tp_new = (newfunc)Partial::create,
    // .tp_init = (initproc)Partial::init,
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

