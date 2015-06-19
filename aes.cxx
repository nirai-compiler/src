#include <Python.h>

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>

static bool _inited = false;

static void _init()
{
    if (!_inited)
    {
        _inited = true;
        ERR_load_crypto_strings();
        OpenSSL_add_all_algorithms();
        OPENSSL_config(NULL);
    }
}

int _handle_error()
{
    ERR_print_errors_fp(stderr);
    return -1;
}

int AES_encrypt(unsigned char* data, int size, unsigned char* key, unsigned char* iv, unsigned char* ciphertext)
{
    EVP_CIPHER_CTX *ctx;
    int ciphertext_len, len;

    if (!(ctx = EVP_CIPHER_CTX_new()))
        return _handle_error();

    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv) != 1)
        return _handle_error();

    if (EVP_EncryptUpdate(ctx, ciphertext, &len, data, size) != 1)
        return _handle_error();

    ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, &ciphertext[len], &len) != 1)
        return _handle_error();

    EVP_CIPHER_CTX_free(ctx);
    return ciphertext_len + len;
}

int AES_decrypt(unsigned char* data, int size, unsigned char* key, unsigned char* iv, unsigned char* plaintext)
{
    EVP_CIPHER_CTX *ctx;
    int plaintext_len, len;

    if (!(ctx = EVP_CIPHER_CTX_new()))
        return _handle_error();

    if (EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv) != 1)
        return _handle_error();

    if (EVP_DecryptUpdate(ctx, plaintext, &len, data, size) != 1)
        return _handle_error();

    plaintext_len = len;

    if(EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1)
        return _handle_error();

    EVP_CIPHER_CTX_free(ctx);
    return plaintext_len + len;
}

static PyObject* py_aes_encrypt(PyObject* self, PyObject* args)
{
    unsigned char* data;
    unsigned char* key;
    unsigned char* iv;
    int size, keysize, ivsize;

    if (!PyArg_ParseTuple(args, "s#s#s#", &data, &size, &key, &keysize, &iv, &ivsize))
    {
        return NULL;
    }

    if (ivsize != 16 || keysize != 16)
    {
        PyErr_Format(PyExc_ValueError, "iv and key must 16 bytes");
        return NULL;
    }

    unsigned char* ciphertext = new unsigned char[size + 16];

    int ciphertext_len = AES_encrypt(data, size, key, iv, ciphertext);
    if (ciphertext_len == -1)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    PyObject* v = Py_BuildValue("s#", ciphertext, ciphertext_len);
    delete[] ciphertext;

    return v;
};

static PyObject* py_aes_decrypt(PyObject* self, PyObject* args)
{
    unsigned char* data;
    unsigned char* key;
    unsigned char* iv;
    int size, keysize, ivsize;

    if (!PyArg_ParseTuple(args, "s#s#s#", &data, &size, &key, &keysize, &iv, &ivsize))
    {
        return NULL;
    }

    if (ivsize != 16 || keysize != 16)
    {
        PyErr_Format(PyExc_ValueError, "iv and key must 16 bytes");
        return NULL;
    }

    unsigned char* plaintext = new unsigned char[size + 16];

    int plaintext_len = AES_decrypt(data, size, key, iv, plaintext);
    if (plaintext_len == -1)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    PyObject* v = Py_BuildValue("s#", plaintext, plaintext_len);
    delete[] plaintext;

    return v;
};

static PyMethodDef Methods[] = {
    {"encrypt", py_aes_encrypt, METH_VARARGS},
    {"decrypt", py_aes_decrypt, METH_VARARGS},
    {NULL, NULL, 0}
};

#ifdef BUILD_PYD
extern "C" __declspec(dllexport)
#endif

void initaes(void)
{
    _init();
    Py_InitModule("aes", Methods);
}
