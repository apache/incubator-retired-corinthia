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

// Abstraction level 1

int DFGetFile(const char *concrete, const char *abstract, DFError **error);
int DFPutFile(const char *concrete, const char *abstract, DFError **error);
int DFCreateFile(const char *concrete, const char *abstract, DFError **error);
              

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

static PyObject* put_func(PyObject* self, PyObject* args)
{
    DFError *error = NULL;
    
    char * concrete = NULL;
    char * abstract = NULL;
    
    if (!PyArg_ParseTuple(args, "ss", &concrete, &abstract)) {
        return NULL;
    }    
    
    if (DFPutFile(concrete, abstract, &error)) {
        Py_RETURN_TRUE;
    }

    fprintf(stderr,"%s\n",DFErrorMessage(&error));
    DFErrorRelease(error);
    Py_RETURN_FALSE;
}

static PyObject* create_func(PyObject* self, PyObject* args)
{
    DFError *error = NULL;
    
    char * concrete = NULL;
    char * abstract = NULL;
    
    if (!PyArg_ParseTuple(args, "ss", &concrete, &abstract)) {
        return NULL;
    }    
    
    if (DFCreateFile(concrete, abstract, &error)) {
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
     {"put", put_func, METH_VARARGS, "Update an existing Word document based on a modified HTML file."},
     {"create", create_func, METH_VARARGS, "Create a new Word document from a HTML file. The Word document must not already exist."},
     {NULL, NULL, 0, NULL}
};


/* module initialization */
PyMODINIT_FUNC

initdfconvert(void)
{
     (void) Py_InitModule("dfconvert", dfconvertMethods);
}
