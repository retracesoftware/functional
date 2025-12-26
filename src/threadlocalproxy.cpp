#include "functional.h"
#include <structmember.h>
#include <sys/resource.h>

struct ThreadLocalProxy : public PyObject {
    Py_tss_t tls;
    vectorcallfunc vectorcall;
    PyObject * error;
    
    static int init(ThreadLocalProxy * proxy, PyObject * args, PyObject * kwds) {
        static const char *kwlist[] = { "error", NULL};

        PyObject * error = nullptr;

        if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O", (char **)kwlist, &error))
        {
            return -1; // Return NULL on failure
        }

        proxy->tls = Py_tss_NEEDS_INIT;
        proxy->vectorcall = (vectorcallfunc)call;
        proxy->error = Py_XNewRef(error);

        if (PyThread_tss_create(&proxy->tls) != 0) {
            PyErr_SetString(PyExc_RuntimeError, "failed to create TSS key");
            return -1;
        }
        return 0;
    }
    
    PyObject * get() { return reinterpret_cast<PyObject *>(PyThread_tss_get(&tls)); }

    PyObject * set(PyObject * target) {
        PyObject *prev = get();
    
        PyThread_tss_set(&tls, target == Py_None ? nullptr : Py_NewRef(target));

        return prev ? prev : Py_NewRef(Py_None);
    }

    static void dealloc(ThreadLocalProxy *proxy) {
       // Clear thread-local values is not possible, but free the key
        PyThread_tss_delete(&proxy->tls);
        Py_XDECREF(proxy->error);
        Py_TYPE(proxy)->tp_free((PyObject *)proxy);
    }

    PyObject * target() {
        PyObject * target = get();
        
        if (!target) {
            PyErr_SetNone(error ? error : PyExc_RuntimeError);
            return nullptr;
        }
        return target;
    }

    static PyObject * getattro(ThreadLocalProxy *self, PyObject *name) {
        PyObject * obj = self->target();
        return obj ? PyObject_GetAttr(obj, name) : nullptr;
    }

    static int setattro(ThreadLocalProxy *self, PyObject * name, PyObject *value) {
        PyObject * obj = self->target();
        return obj ? PyObject_SetAttr(obj, name, value) : -1;
    }

    static PyObject * call(ThreadLocalProxy * self, PyObject** args, size_t nargsf, PyObject* kwnames) {
        PyObject * obj = self->target();
        return obj ? PyObject_Vectorcall(obj, args, nargsf, kwnames) : nullptr;
    }

    static PyObject * repr(ThreadLocalProxy *self) {
        PyObject * target = self->get();
        if (!target) {
            return PyUnicode_FromString("<ThreadLocalProxy: unset>");
        }
        return PyObject_Repr(target);
    }

    static PyObject * str(ThreadLocalProxy *self) {
        PyObject * target = self->get();
        if (!target) {
            return PyUnicode_FromString("<ThreadLocalProxy: unset>");
        }
        return PyObject_Str(target);
    }

    static PyObject * iter(ThreadLocalProxy *self) {
        PyObject * obj = self->target();
        return obj ? PyObject_GetIter(obj) : nullptr;
    }

    static PyObject * iternext(ThreadLocalProxy *self) {
        PyObject * obj = self->target();
        return obj ? PyIter_Next(obj) : nullptr;
    }

    static PyObject * set_classmethod(PyObject *cls, PyObject *args, PyObject * kwds) {

        ThreadLocalProxy * self;
        PyObject * target;

        static const char *kwlist[] = {"proxy", "target", NULL};

        if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!O", (char **)kwlist, &ThreadLocalProxy_Type, &self, &target))
        {
            return nullptr; // Return NULL on failure
        }
        return self->set(target);
    }

    static PyObject * get_classmethod(PyObject *cls, PyObject *args, PyObject * kwds) {

        ThreadLocalProxy * self;

        static const char *kwlist[] = {"proxy", NULL};

        if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!", (char **)kwlist, &ThreadLocalProxy_Type, &self))
        {
            return nullptr; // Return NULL on failure
        }

        PyObject * obj = self->get();
        return Py_NewRef(obj ? obj : Py_None);
    }
};

static PyMethodDef methods[] = {
    {"set", (PyCFunction)ThreadLocalProxy::set_classmethod, METH_VARARGS | METH_KEYWORDS | METH_CLASS, "Set the thread-local target"},
    {"get", (PyCFunction)ThreadLocalProxy::get_classmethod, METH_VARARGS | METH_KEYWORDS | METH_CLASS, "Set the thread-local target"},
    {NULL, NULL, 0, NULL}
};

PyTypeObject ThreadLocalProxy_Type = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = MODULE "ThreadLocalProxy",
    .tp_basicsize = sizeof(ThreadLocalProxy),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)ThreadLocalProxy::dealloc,
    .tp_vectorcall_offset = OFFSET_OF_MEMBER(ThreadLocalProxy, vectorcall),
    .tp_repr = (reprfunc)ThreadLocalProxy::repr,
    .tp_call = PyVectorcall_Call,
    .tp_str = (reprfunc)ThreadLocalProxy::str,
    .tp_getattro = (getattrofunc)ThreadLocalProxy::getattro,
    .tp_setattro = (setattrofunc)ThreadLocalProxy::setattro,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_VECTORCALL,
    .tp_doc = "TODO",
    .tp_iter = (getiterfunc)ThreadLocalProxy::iter,
    .tp_iternext = (iternextfunc)ThreadLocalProxy::iternext,
    // .tp_methods = methods,
    // .tp_members = members,
    .tp_methods = methods,
    .tp_init = (initproc)ThreadLocalProxy::init,
    .tp_new = PyType_GenericNew,
};
