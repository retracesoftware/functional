#include "functional.h"
#include "object.h"
#include "pyerrors.h"
#include "unordered_dense.h"

using namespace ankerl::unordered_dense;

struct Cache {
    PyObject_HEAD
    map<PyObject *, PyObject *> m_cache;
    PyObject * m_lookup;
    vectorcallfunc vectorcall;

    int traverse(visitproc visit, void* arg) {
        Py_VISIT(m_lookup);

        for (auto it = m_cache.begin(); it != m_cache.end(); it++) {
            Py_VISIT(it->first);
            Py_VISIT(it->second);
        }

        return 0;
    }

    int clear() {
        for (auto it = m_cache.begin(); it != m_cache.end(); it++) {
            Py_CLEAR(it->first);
            Py_CLEAR(it->second);
        }
        Py_CLEAR(m_lookup);
        return 0;
    }

    PyObject * call(PyObject * obj) {
        auto it = m_cache.find(obj);

        if (it == m_cache.end()) {
            PyObject * item = PyObject_CallOneArg(m_lookup, obj);

            if (!item) return nullptr;

            if (item != Py_None)
                m_cache[Py_NewRef(obj)] = item;

            return Py_NewRef(item);
        } else {
            return Py_NewRef(it->second);
        }
    }

    Cache(PyObject * lookup, vectorcallfunc vectorcall) : m_cache(), m_lookup(Py_NewRef(lookup)), vectorcall(vectorcall)  {}
    ~Cache() {}
};

static PyMethodDef methods[] = {
    // {"write_getattr", (PyCFunction)write_getattr, METH_FASTCALL, "TODO"},
    // {"write_call", (PyCFunction)write_call, METH_FASTCALL | METH_KEYWORDS, "TODO"},
    // {"write_result", (PyCFunction)write_result, METH_O, "TODO"},
    // {"write_error", (PyCFunction)write_error, METH_FASTCALL, "TODO"},
    // {"write_new", (PyCFunction)write_new, METH_O, "TODO"},
    // {"write_new_patch_type", (PyCFunction)write_new_patch_type, METH_O, "TODO"},
    // {"close", (PyCFunction)close_impl, METH_NOARGS, "TODO"},
    // {"flush", (PyCFunction)flush_impl, METH_NOARGS, "TODO"},
    {NULL}  // Sentinel
};

static PyObject * vectorcall(Cache * self, PyObject** args, size_t nargsf, PyObject* kwnames) {
    int nargs = PyVectorcall_NARGS(nargsf);

    if (nargs != 1 || kwnames) {
        PyErr_SetString(PyExc_TypeError, "Cache takes a single positional parameter");
    }
    return self->call(args[0]);
}

static PyObject* create(PyTypeObject* type, PyObject* args, PyObject* kwds) {

    PyObject * lookup;
    
    static const char* kwlist[] = {"lookup", NULL};  // Keywords allowed

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", (char **)kwlist, &lookup)) {
        return NULL;  // Return NULL to propagate the parsing error
    }

    Cache* self = (Cache *)type->tp_alloc(type, 0);

    if (!self) {
        return NULL;
    }

    try {
        new (self) Cache(lookup, reinterpret_cast<vectorcallfunc>(vectorcall));
    } catch (...) {
        Py_DECREF(self);
        return NULL;
    }

    return (PyObject *)self;
}

static void dealloc(Cache *self) {
    self->~Cache();
    PyObject_GC_UnTrack(self);
    Py_TYPE(self)->tp_free((PyObject *)self);  // Free the object
}

static int traverse(Cache* self, visitproc visit, void* arg) {
    return self->traverse(visit, arg);
}

static int clear(Cache* self) {
    return self->clear();
}

PyTypeObject Cache_Type = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = MODULE "Cache",
    .tp_basicsize = sizeof(Cache),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)dealloc,
    .tp_vectorcall_offset = offsetof(Cache, vectorcall),
    .tp_call = PyVectorcall_Call,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
    .tp_doc = "Returns id, None if no id found",
    .tp_traverse = (traverseproc)traverse,
    .tp_clear = (inquiry)clear,
    .tp_methods = methods,
    .tp_new = create,
};