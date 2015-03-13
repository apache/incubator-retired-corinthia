#include <Python.h>

typedef struct DFError DFError;
void DFErrorRelease(DFError *error);
const char *DFErrorMessage(DFError **error);

int DFGetFile(const char *concreteFilename,
              const char *abstractFilename,
              DFError **error);

static PyObject* get_func(PyObject* self, PyObject* args)
{
    DFError *error = NULL;
    
    char * concrete = NULL;
    char * abstract = NULL;
    
    if (!PyArg_ParseTuple(args, "ss", &concrete, &abstract)) {
        return NULL;
    }    
    
    if (DFGetFile(concrete, abstract, &error)) {
        Py_RETURN_TRUE;
    }

    fprintf(stderr,"%s\n",DFErrorMessage(&error));
    DFErrorRelease(error);
    Py_RETURN_FALSE;
}

/*  define functions in module */
static PyMethodDef dfconvertMethods[] =
{
     {"get", get_func, METH_VARARGS, "Create a new HTML file from input document"},
     {NULL, NULL, 0, NULL}
};


/* module initialization */
PyMODINIT_FUNC

initdfconvert(void)
{
     (void) Py_InitModule("dfconvert", dfconvertMethods);
}
