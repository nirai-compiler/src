#pragma once

#include "windows.h"
#include <pandabase.h>

#undef NDEBUG

#include <Python.h>
#include <iostream>
#include <fstream>
#include <vector>

#define NIRAI_VERSION_MAJOR 1
#define NIRAI_VERSION_MINOR 6

// fwd decl nirai methods

int AES_encrypt(unsigned char* data, int size, unsigned char* key,
                unsigned char* iv, unsigned char* ciphertext);

int AES_decrypt(unsigned char* data, int size, unsigned char* key,
                unsigned char* iv, unsigned char* plaintext);

// fwd decl niraicall methods
// these methods *must* be defined by the project file

int niraicall_onPreStart(int argc, char* argv[]);
int niraicall_onLoadGameData();
extern "C" PyObject* niraicall_deobfuscate(char*, Py_ssize_t);

// N. B. the char* argument passed to niraicall_deobfuscate shall not be edited or deallocated!
