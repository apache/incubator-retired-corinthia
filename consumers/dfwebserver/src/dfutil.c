#include <Python.h>

typedef struct DFError DFError;
void DFErrorRelease(DFError *error);
const char *DFErrorMessage(DFError **error);

int normalizeFile(const char *filename, DFError **error);

static PyObject* normalize_func(PyObject* self, PyObject* args)
{
    DFError *dferr = NULL;
    
    char * filename = NULL;
    
    if (!PyArg_ParseTuple(args, "s", &filename)) {
        return NULL;
        /* if the above function returns -1, an appropriate Python exception will
        * have been set, and the function simply returns NULL
        */
    }
    
    if (filename!=NULL) {
        if (1 == normalizeFile(filename, &dferr)) {
            Py_RETURN_TRUE;
        }
        fprintf(stderr,"%s\n",DFErrorMessage(&dferr));
        DFErrorRelease(dferr);
    }
    
    Py_RETURN_FALSE;
}


/*  define functions in module */
static PyMethodDef dfutilMethods[] =
{
     {"normalize", normalize_func, METH_VARARGS, "evaluate the sine"},
     {NULL, NULL, 0, NULL}
};

/* module initialization */
PyMODINIT_FUNC

initdfutil(void)
{
     (void) Py_InitModule("dfutil", dfutilMethods);
}
