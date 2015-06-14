#include <Python.h>
#include <string>
#include <iostream>

/*
Usage:

C++:
    Make a forward declaration for rc4 method, eg: string rc4(const char* data, const char* key, int ds, int ks)
    Link your app against this module
    
Python:
    Generate a PYD (within Nirai.exe you can just import it)
    
    >>> import rc4
    >>> rc4.rc4_setkey('Test123')
    >>> string = "HELLO WORLD!"
    >>> enc = rc4.rc4(string)
    >>> dec = rc4.rc4(enc)
    >>> print dec
    HELLO WORLD!
*/

using namespace std;

char* g_python_key = "Nirai";
int g_python_key_size = 5;

string rc4(const char* data, const char* key, int ds, int ks)
{
    unsigned int i, j, k;
    
    unsigned char s[256];
    for (i = 0; i < 256; i++)
    {
        s[i] = i;
    };

    unsigned char temp;
    
    j = 0;
    for (i = 0; i < 256; i++)
    {
        j = (j + s[i] + key[i % ks]) % 256;
        temp = s[i];
        s[i] = s[j];
        s[j] = temp;
    };

    j = 0;
    i = 0;
    string out;
    
    for (k = 0; k < ds; k++)
    {
        j = (j + 1) % 256;
        i = (i + s[j]) % 256;
        
        temp = s[i];
        s[i] = s[j];
        s[j] = temp;   
		out += (unsigned char)(data[k] ^ s[(s[j] + s[i]) % 256]);
    };
    
	return out;
};

static PyObject* py_rc4(PyObject* self, PyObject* args)
{
    const char* _data;
    int _datalen;
    
    if (!PyArg_ParseTuple(args, "s#:in_bytes", &_data, &_datalen))
    {
        return NULL;
    };
    
    string res = rc4(_data, g_python_key, _datalen, g_python_key_size);
  
    PyObject* v = Py_BuildValue("s#", res.c_str(), res.size());
    res.erase();
    return v;
};

static PyObject* py_rc4_setkey(PyObject* self, PyObject* args)
{    
    if (!PyArg_ParseTuple(args, "s#:in_bytes", &g_python_key, &g_python_key_size))
    {
        return NULL;
    };

    Py_INCREF(Py_None);
    return Py_None;
};

static PyMethodDef Methods[] = {
    {"rc4", py_rc4, METH_VARARGS},
    {"rc4_setkey", py_rc4_setkey, METH_VARARGS},
    {NULL, NULL, 0}
};

#ifdef RC4_BUILD_PYD
extern "C" __declspec(dllexport)
#endif

void initrc4(void)
{
    PyObject *m;

    m = Py_InitModule("rc4", Methods);
    if (m == NULL)
        return;
}
