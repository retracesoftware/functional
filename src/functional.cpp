#include "functional.h"
#include "object.h"

static PyObject * apply_impl(PyObject *self, PyObject *const *args, Py_ssize_t nargsf, PyObject *kwnames) {
    size_t nargs = PyVectorcall_NARGS(nargsf);

    PyObject * func = args[0];

    PyObject * result = PyObject_Vectorcall(func, args + 1, (nargs - 1) | PY_VECTORCALL_ARGUMENTS_OFFSET, kwnames);

    ((PyObject **)args)[0] = func;

    return result;
}

static PyObject * first_arg_impl(PyObject *self, PyObject *const *args, Py_ssize_t nargsf, PyObject *kwnames) {
    if (PyVectorcall_NARGS(nargsf) == 0) {
        PyErr_SetString(PyExc_TypeError, "first_arg() requires at least one positional argument");
        return nullptr;
    }
    return Py_NewRef(args[0]);
}

// static PyObject * partial_impl(PyObject *self, PyObject *const *args, Py_ssize_t nargs) {

//     if (nargs < 2) {
//         PyErr_SetString(PyExc_TypeError, "partial() requires at least two arguments");
//         return NULL;
//     }
//     return partial(args[0], args + 1, nargs - 1);
// }

static PyObject * dispatch_impl(PyObject *self, PyObject *const *args, Py_ssize_t nargs) {
    return dispatch(args + 1, nargs - 1);
}

static PyObject * firstof_impl(PyObject *self, PyObject *const *args, Py_ssize_t nargs) {
    return firstof(args, nargs);
}

static PyObject * py_typeof(PyObject *self, PyObject *obj) { return Py_NewRef((PyObject *)Py_TYPE(obj)); }

static PyObject * identity(PyObject *self, PyObject *obj) { return Py_NewRef(obj); }

static PyObject * py_instanceof(PyObject *self, PyObject * args, PyObject *kwds) { 
    PyTypeObject * cls = nullptr;
    PyTypeObject * andnot = nullptr;

    static const char *kwlist[] = {"cls", "andnot", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!|O!", (char **)kwlist, 
        &PyType_Type, &cls, &PyType_Type, &andnot))
    {
        return nullptr; // Return NULL on failure
    }
    
    if (andnot) {
        return instanceof_andnot(cls, andnot);
    } else {
        return instanceof(cls);
    }
}

static PyObject * py_instance_test(PyObject *self, PyObject *obj) { 
    if (!PyType_Check(obj)) {
        PyErr_Format(PyExc_TypeError, "instance_test must be passed a type, was passed: %S", obj);
        return nullptr;
    }
    return instance_test(reinterpret_cast<PyTypeObject *>(obj));
}

static PyObject * py_notinstance_test(PyObject *self, PyObject *obj) { 
    if (!PyType_Check(obj)) {
        PyErr_Format(PyExc_TypeError, "notinstance_test must be passed a type, was passed: %S", obj);
        return nullptr;
    }
    return notinstance_test(reinterpret_cast<PyTypeObject *>(obj));
}

// Module-level methods
static PyMethodDef module_methods[] = {
    {"isinstanceof", (PyCFunction)py_instanceof, METH_VARARGS | METH_KEYWORDS, "TODO"},
    {"instance_test", (PyCFunction)py_instance_test, METH_O, "TODO"},
    {"notinstance_test", (PyCFunction)py_notinstance_test, METH_O, "TODO"},
    {"typeof", (PyCFunction)py_typeof, METH_O, "TODO"},
    {"identity", (PyCFunction)identity, METH_O, "TODO"},
    {"apply", (PyCFunction)apply_impl, METH_FASTCALL | METH_KEYWORDS, "TODO"},
    {"first_arg", (PyCFunction)first_arg_impl, METH_FASTCALL | METH_KEYWORDS, "TODO"},
    // {"partial", (PyCFunction)partial_impl, METH_FASTCALL, "TODO"},
    {"dispatch", (PyCFunction)dispatch_impl, METH_FASTCALL, "TODO"},
    {"firstof", (PyCFunction)firstof_impl, METH_FASTCALL, "TODO"},
    {NULL, NULL, 0, NULL}  // Sentinel
};

// Module definition
static PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "retracesoftware.functional",
    "Example module with a dynamic type",
    0,
    module_methods
};

PyObject *ThreadLocalError = NULL;

// Module initialization
PyMODINIT_FUNC PyInit_retracesoftware_functional(void) {
    PyObject* module;

    // Create the module
    module = PyModule_Create(&moduledef);
    if (!module) {
        return NULL;
    }

    ThreadLocalError = PyErr_NewException(MODULE "ThreadLocalError", PyExc_RuntimeError, NULL);
    if (!ThreadLocalError) return nullptr;

    PyTypeObject * hidden_types[] = {
        &FirstOf_Type,
        &InstanceTest_Type,
        nullptr
    };

    for (int i = 0; hidden_types[i]; i++) {
        PyType_Ready(hidden_types[i]);
    }

    PyTypeObject * types[] = {
        &CallAll_Type,
        &Observer_Type,
        &Compose_Type,
        &SideEffect_Type,
        &Repeatedly_Type,
        &ManyPredicate_Type,
        &NotPredicate_Type,
        &AndPredicate_Type,
        &OrPredicate_Type,
        &TypePredicate_Type,
        &TransformArgs_Type,
        &First_Type,
        &Advice_Type,
        &WhenPredicate_Type,
        &CasePredicate_Type,
        &Memoize_Type,
        &Cache_Type,
        &ThreadLocalProxy_Type,
        &Partial_Type,
        &MethodInvoker_Type,
        &Intercept_Type,
        &Indexer_Type,
        &Param_Type,
        &TernaryPredicate_Type,
        &IfThenElse_Type,
        &AnyArgs_Type,
        &Walker_Type,
        &Always_Type,
        &SelfApply_Type,
        &TransformApply_Type,
        &Constantly_Type,
        // &When_Type,

        // &WhenNot_Type,
        NULL
    };
        
    for (int i = 0; types[i]; i++) {
        PyType_Ready(types[i]);

        // Find the last dot in the string
        const char *last_dot = strrchr(types[i]->tp_name, '.');

        // If a dot is found, the substring starts after the dot
        const char *name = (last_dot != NULL) ? (last_dot + 1) : types[i]->tp_name;

        PyModule_AddObject(module, name, (PyObject *)types[i]);
        // Py_DECREF(types[i]);
    }
    return module;
}
