#include "nirai.h"

#include "graphicsPipeSelection.h"
#include "config_openalAudio.h"

AudioManager* Create_OpenALAudioManager();
// Register OpenAL audio (like setup_env, must be in static time).
void* load_openal()
{
    init_libOpenALAudio();
    AudioManager::register_AudioManager_creator(Create_OpenALAudioManager);
    return NULL;
}

static void* __ = load_openal();

// P3D Python modules initers.
#ifdef WIN32
#define _P3D_INIT(MODULE) extern "C" __declspec(dllexport) void init##MODULE ();

#else
#define _P3D_INIT(MODULE) extern "C" void init##MODULE ();

#endif

_P3D_INIT(_core)
_P3D_INIT(_direct)
_P3D_INIT(fx)
_P3D_INIT(egg)
_P3D_INIT(ode)
_P3D_INIT(physics)
_P3D_INIT(interrogatedb)

// P3D CXX fwd decls.
void init_libwgldisplay();
void init_libmovies();
void init_libpnmimagetypes();

static PyMethodDef NiraiMethods[] = {{NULL, NULL, 0}};

static void inject_into_sys_modules(const std::string& module, const std::string& alias)
{    
    PyObject* sysmodule = PyImport_ImportModule("sys");
    Py_INCREF(sysmodule);
    PyObject* modulesdict = PyObject_GetAttrString(sysmodule, "modules");
    Py_INCREF(modulesdict);
    
    PyObject* mod = PyImport_ImportModule(module.c_str());
    Py_INCREF(mod);
    PyDict_SetItemString(modulesdict, alias.c_str(), mod);
    
    Py_DECREF(modulesdict);
    Py_DECREF(sysmodule);
}

static void start_nirai()
{
    // Setup __nirai__.
    PyObject* niraimod = Py_InitModule("__nirai__", NiraiMethods);
    PyObject* bt = PyImport_ImportModule("__builtin__");
    PyObject_SetAttrString(bt, "__nirai__", niraimod);
    Py_INCREF(niraimod);
    Py_DECREF(bt);

    // Init Panda3D.
    PyObject* panda3d_mod = Py_InitModule("panda3d", NiraiMethods);
    Py_INCREF(panda3d_mod);

    init_core();

    // Setup the display.
    init_libwgldisplay();

    // Setup audio.
    init_libmovies();

    // Setup pnmimagetypes.
    init_libpnmimagetypes();

    // Init other modules.
    initinterrogatedb();
    init_direct();
    initegg();
    initfx();
    initode();
    initphysics();
    
    // Remove our hacked panda3d root from sys.modules
    // so it can be reloaded with a proper __init__.py
    // but all panda3d.xxx submodules are still accessible.
    // However, another hack is required (see main).
    PyObject* sysmodule = PyImport_ImportModule("sys");
    Py_INCREF(sysmodule);
    PyObject* modulesdict = PyObject_GetAttrString(sysmodule, "modules");
    Py_INCREF(modulesdict);
    PyDict_DelItemString(modulesdict, "panda3d");
    Py_DECREF(modulesdict);
    Py_DECREF(sysmodule);
};

// fwd decls
void initaes();

static void setup_python()
{
    initaes();

    // Clear sys.path.
    PyObject* sysmodule = PyImport_ImportModule("sys");
    Py_INCREF(sysmodule);
    
    PyObject* pathlist = PyObject_GetAttrString(sysmodule, "path");
    Py_DECREF(pathlist);
    
    PyObject* newpathlist = PyList_New(1);
    Py_INCREF(newpathlist);
    PyList_SET_ITEM(newpathlist, 0, Py_BuildValue("s", "."));
    PyObject_SetAttrString(sysmodule, "path", newpathlist);
    
    Py_DECREF(sysmodule);
}

int main(int argc, char* argv[])
{
    if (niraicall_onPreStart(argc, argv))
        return 1;

    Py_NoSiteFlag++;
    Py_FrozenFlag++;
    Py_InspectFlag++; // For crash at the exit.

    Py_SetProgramName(argv[0]);
    Py_Initialize();
    PyEval_InitThreads();
    PySys_SetArgv(argc, argv);

    start_nirai();
    setup_python();

    if (niraicall_onLoadGameData())
        return 2;
    
    // Until panda3d directory stops mixing .py and .pyd files, we need to explicitly do this:
    // N.B. No error checking, these modules are guaranteed to exist.
    PyObject* panda3d_mod = PyImport_ImportModule("panda3d");
    PyObject_SetAttrString(panda3d_mod, "interrogatedb", PyImport_ImportModule("panda3d.interrogatedb"));
    PyObject_SetAttrString(panda3d_mod, "_direct", PyImport_ImportModule("panda3d._direct"));
    PyObject_SetAttrString(panda3d_mod, "egg", PyImport_ImportModule("panda3d.egg"));
    PyObject_SetAttrString(panda3d_mod, "fx", PyImport_ImportModule("panda3d.fx"));
    PyObject_SetAttrString(panda3d_mod, "ode", PyImport_ImportModule("panda3d.ode"));
    PyObject_SetAttrString(panda3d_mod, "physics", PyImport_ImportModule("panda3d.physics"));

    PyObject* res = PyImport_ImportModule("NiraiStart");

    if (res == NULL)
    {
        std::cerr << "Error importing NiraiStart!" << std::endl;
        PyErr_Print();

        return 3;
    }

    return 0;
}
