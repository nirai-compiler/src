#pragma once

#include "windows.h"
#include <pandabase.h>

#undef NDEBUG

#include <Python.h>
#include <iostream>
#include <fstream>
#include <vector>

#define NIRAI_VERSION_MAJOR 1
#define NIRAI_VERSION_MINOR 5

// fwd decl niraicall methods
// these methods *must* be defined by the project file

int niraicall_onPreStart(int argc, char* argv[]);
int niraicall_onLoadGameData();
