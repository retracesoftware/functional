#include "functional.h"
#include "object.h"

static PyObject * apply_impl(PyObject *self, PyObject *const *args, Py_ssize_t nargsf, PyObject *kwnames) {
    size_t nargs = PyVectorcall_NARGS(nargsf);

    PyObject * func = args[0];

    PyObject * result = PyObject_Vectorcall(func, args + 1, (nargs - 1) | PY_VECTORCALL_ARGUMENTS_OFFSET, kwnames);

    ((PyObject **)args)[0] = func;

    return result;
}

static PyObject * partial_impl(PyObject *self, PyObject *const *args, Py_ssize_t nargs) {

    if (nargs < 2) {
        PyErr_SetString(PyExc_TypeError, "partial() requires at least two arguments");
        return NULL;
    }
    return partial(args[0], args + 1, nargs - 1);
}

static PyObject * identity(PyObject *self, PyObject *obj) { return Py_NewRef(obj); }

// Module-level methods
static PyMethodDef module_methods[] = {
    {"identity", (PyCFunction)identity, METH_O, "TODO"},
    {"apply", (PyCFunction)apply_impl, METH_FASTCALL | METH_KEYWORDS, "TODO"},
    {"partial", (PyCFunction)partial_impl, METH_FASTCALL, "TODO"},
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
PyMODINIT_FUNC PyInit_functional(void) {
    PyObject* module;

    // Create the module
    module = PyModule_Create(&moduledef);
    if (!module) {
        return NULL;
    }

    ThreadLocalError = PyErr_NewException(MODULE "ThreadLocalError", PyExc_RuntimeError, NULL);
    if (!ThreadLocalError) return nullptr;

    PyTypeObject * types[] = {
        &CallAll_Type,
        &TransformCall_Type,
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
        // &Walker_Type,
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
