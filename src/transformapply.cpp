#include "functional.h"

struct Pair {
    vectorcallfunc vectorcall;
    PyObject * callable;
};

struct TransformApply : public PyVarObject {
    vectorcallfunc vectorcall;
    // std::vector<std::pair<PyTypeObject *, PyObject *>> dispatch;
    PyObject * function; 
    bool short_curcuit;       
    vectorcallfunc function_vectorcall;
    Pair transforms[];

    static int clear(TransformApply* self) {
        Py_CLEAR(self->function);
        for (int i = 0; i < self->ob_size; i++) {
            Py_CLEAR(self->transforms[i].callable);
        }
        return 0;
    }
    
    static int traverse(TransformApply* self, visitproc visit, void* arg) {
        Py_VISIT(self->function);
        for (int i = 0; i < self->ob_size; i++) {
            Py_VISIT(self->transforms[i].callable);
        }
        return 0;
    }
    
    static void dealloc(TransformApply *self) {

        PyObject_GC_UnTrack(self);          // Untrack from the GC

        clear(self);
        Py_TYPE(self)->tp_free((PyObject *)self);  // Free the object
    }

    static PyObject * call(TransformApply * self, PyObject* const* args, size_t nargsf, PyObject* kwnames) {

        if (kwnames) {
            PyErr_Format(PyExc_TypeError, "%S does not currently support keyword arguments", Py_TYPE(self));
            return nullptr;
        }

        Py_ssize_t nargs = PyVectorcall_NARGS(nargsf);

        if (nargs == 0) {
            PyErr_Format(PyExc_TypeError, "at least one positional arg must be passed to %S", self);
            return nullptr;
        }

        PyObject ** mem = (PyObject **)alloca(sizeof(PyObject *) * nargs) + 1;

        // if (nargs < self->ob_size) {
            


        //     PyErr_Format(PyExc_TypeError, "insufficient number of args: %i passed to %S", nargs, self);
        //     return nullptr;

        // } else if (nargs > self->ob_size) {


        // } else {


        // }

        if (self->short_curcuit) {
            for (int i = 0; i < self->ob_size; i++) {
                if (args[i] == Py_None) {
                    Py_RETURN_NONE;
                }
            }
        }

        size_t total_args = nargs > self->ob_size ? nargs : self->ob_size;


        for (int i = 0; i < self->ob_size; i++) {

            if (self->transforms[i].vectorcall) {
                mem[i] = self->transforms[i].vectorcall(self->transforms[i].callable, args + i, 1, nullptr);
                if (!mem[i]) {
                    for (int j = 0; j < i; j++) {
                        Py_DECREF(mem[j]);
                    }
                    return nullptr;
                } else if (self->short_curcuit && mem[i] == Py_None) {
                    for (int j = 0; j < i; j++) {
                        Py_DECREF(mem[j]);
                    }
                    Py_RETURN_NONE;
                }
            } else {
                mem[i] = Py_NewRef(args[i]);
            }
        }

        if (self->ob_size > nargs) {

        }

        for (int i = self->ob_size; i < total_args; i++) {
            mem[i] = Py_NewRef(args[i]);
        }

        PyObject * result = self->function_vectorcall(self->function, mem, nargs | PY_VECTORCALL_ARGUMENTS_OFFSET, nullptr);

        for (int i = 0; i < total_args; i++) {
            Py_DECREF(mem[i]);
        }
        return result;
    }

    static PyObject* create(PyTypeObject* type, PyObject* args, PyObject* kwds) {
        if (PyTuple_Size(args) == 0) {
            PyErr_SetString(PyExc_TypeError, "partial requires at least one positional argument");
            return nullptr;
        }

        // Use PyObject_NewVar to allocate memory for the object
        // Partial* self = (Partial *)Partial_Type.tp_alloc(&Partial_Type, PyTuple_Size(args) - 1);
        TransformApply* self = (TransformApply *)type->tp_alloc(type, PyTuple_Size(args) - 1);

        // Check if the allocation was successful
        if (self == NULL) {
            return NULL; // Return NULL on error
        }
        
        self->short_curcuit = false;

        PyObject * sc = kwds ? PyDict_GetItemString(kwds, "short_curcuit") : nullptr;
        if (sc && sc == Py_True) {
            self->short_curcuit = true;
        }

        self->function = Py_NewRef(PyTuple_GetItem(args, 0));
        self->function_vectorcall = extract_vectorcall(self->function);

        for (Py_ssize_t i = 0; i < self->ob_size; i++) {
            PyObject * callable = PyTuple_GetItem(args, i + 1);
            if (callable == Py_None) {
                self->transforms[i].callable = nullptr;
                self->transforms[i].vectorcall = nullptr;
            } else {
                self->transforms[i].callable = Py_NewRef(callable);
                self->transforms[i].vectorcall = extract_vectorcall(callable);
            }
        }
        self->vectorcall = (vectorcallfunc)call;

        return (PyObject*)self;
    }

    static PyObject* descr_get(PyObject *self, PyObject *obj, PyObject *type) {
        return obj == NULL || obj == Py_None ? Py_NewRef(self) : PyMethod_New(self, obj);
    }
};

PyTypeObject TransformApply_Type = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = MODULE "transform_apply",
    .tp_basicsize = sizeof(TransformApply),
    .tp_itemsize = sizeof(Pair),
    .tp_dealloc = (destructor)TransformApply::dealloc,
    .tp_vectorcall_offset = OFFSET_OF_MEMBER(TransformApply, vectorcall),
    .tp_call = PyVectorcall_Call,
    .tp_flags = Py_TPFLAGS_DEFAULT | 
                Py_TPFLAGS_HAVE_GC | 
                Py_TPFLAGS_HAVE_VECTORCALL | 
                Py_TPFLAGS_METHOD_DESCRIPTOR |
                Py_TPFLAGS_BASETYPE,
    .tp_doc = "TODO",
    .tp_traverse = (traverseproc)TransformApply::traverse,
    .tp_clear = (inquiry)TransformApply::clear,
    .tp_descr_get = TransformApply::descr_get,

    // .tp_methods = methods,
    // .tp_members = members,
    .tp_new = (newfunc)TransformApply::create,
    // .tp_init = (initproc)Partial::init,
    // .tp_new = PyType_GenericNew,
};

// PyObject * transformapply(PyObject * function, PyObject * const * args, size_t nargs) {

//     TransformApply * self = (TransformApply *)TransformApply_Type.tp_alloc(&TransformApply_Type, nargs);
    
//     if (!self) {
//         return NULL;
//     }

//     self->ob_size = nargs;

//     for (size_t i = 0; i < nargs; i++) {
//         self->args[i] = Py_NewRef(args[i]);
//     }
//     self->vectorcall = (vectorcallfunc)TransformApply::call;
//     self->function = Py_NewRef(function);

//     return (PyObject *)self;
// }

