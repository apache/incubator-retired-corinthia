// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

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
