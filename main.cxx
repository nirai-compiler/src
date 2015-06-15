#include "nirai.h"

#include "graphicsPipeSelection.h"
#include "config_openalAudio.h"

bool does_dir_exist(string pathname)
{
  DWORD ftyp = GetFileAttributesA(pathname.c_str());
  if (ftyp == INVALID_FILE_ATTRIBUTES)
    return false;  // something is wrong with your path!

  if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
    return true;

  return false;
}

void clear_env()
{
    _putenv_s("PANDA_PRC_PATH", "");
    _putenv_s("PANDA_PRC_DIR", "");
    _putenv_s("PRC_PATH", "");
    _putenv_s("PRC_DIR", "");
}

// first: setup the environ
// so we don't load unwanted prcs
// must be done in static time (outside main)
void* setup_env()
{
    static const char garbage[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string badpath = "invalid";
    
    while (does_dir_exist(badpath))
        badpath += garbage[rand() % 15];
        
    _putenv_s("PANDA_PRC_PATH", badpath.c_str());
    _putenv_s("PANDA_PRC_DIR", badpath.c_str());
    _putenv_s("PRC_PATH", badpath.c_str());
    _putenv_s("PRC_DIR", badpath.c_str());
    return NULL;
}

static void* _ = setup_env();

AudioManager* Create_OpenALAudioManager();
// register openal audio (like setup_env, must be in static time)
void* load_openal()
{
    init_libOpenALAudio();
    AudioManager::register_AudioManager_creator(Create_OpenALAudioManager);
    return NULL;
}
 
static void* __ = load_openal();
 
// p3d python modules initers
extern "C" __declspec(dllexport) void initcore();
extern "C" __declspec(dllexport) void init_c_direct();
extern "C" __declspec(dllexport) void initegg();
extern "C" __declspec(dllexport) void initfx();
extern "C" __declspec(dllexport) void initode();
extern "C" __declspec(dllexport) void initphysics();

// p3d cxx fwd decls
void init_libwgldisplay();
void init_libmovies();
void init_libpnmimagetypes();

static PyMethodDef NiraiMethods[] = {{NULL, NULL, 0}};

void inject_into_sys_modules(const std::string& module, const std::string& alias)
{
    std::string cmd = "__import__('sys').modules['";
    cmd += alias;
    cmd += "'] = __import__('";
    cmd += module;
    cmd += "')";
    PyRun_SimpleString(cmd.c_str());
}

void patch_module(PyObject* mod, const char* module, const char* alias, const char* fullname)
{
    PyObject* submodule = PyImport_ImportModule(module);
    PyObject_SetAttrString(mod, alias, submodule);
    inject_into_sys_modules(module, fullname);
}

void patch_module(PyObject* mod, const char* module, const char* alias)
{
    std::string fullname(PyModule_GetName(mod));
    fullname += ".";
    fullname += alias;
    patch_module(mod, module, alias, fullname.c_str());
}

void patch_module(PyObject* mod, const char* module)
{
    patch_module(mod, module, module);
}

void start_nirai()
{
    // setup __nirai__   
    PyObject* niraimod = Py_InitModule("__nirai__", NiraiMethods);
    PyObject* bt = PyImport_ImportModule("__builtin__");
    PyObject_SetAttrString(bt, "__nirai__", niraimod);
    Py_INCREF(niraimod);
    Py_DECREF(bt);
    
    // init panda3d
    initcore();
    clear_env();
    
    // setup the display
    init_libwgldisplay();

    // setup audio
    init_libmovies();
    
    // setup pnmimagetypes
    init_libpnmimagetypes();
    
    // init other modules
    init_c_direct();
    initegg();
    initfx();
    initode();
    initphysics();
    
    // now the modules are loaded we need to fix their internal name
    // because normally they are loaded from panda3d directory
    // but here they lack "panda3d." prefix, which we add manually
    // also, _c_direct requires special alias
    PyObject* panda3d_mod = Py_InitModule("panda3d", NiraiMethods);
    Py_INCREF(panda3d_mod);
    
    patch_module(panda3d_mod, "core");
    patch_module(panda3d_mod, "_c_direct", "direct");
    patch_module(panda3d_mod, "egg");
    patch_module(panda3d_mod, "fx");
    patch_module(panda3d_mod, "ode");
    patch_module(panda3d_mod, "physics");
};

// fwd decls
void initaes();
extern "C" __declspec(dllexport) void initunicodedata();

void setup_python()
{
    // clear sys.path
    PyRun_SimpleString("__import__('sys').path = ['.']");
    
    initaes();
    initunicodedata();
           
    // setup some modules
    inject_into_sys_modules("core", "libpandaexpress");
    // PyRun_SimpleString("__import__('sys').modules['libpandaexpress'] = __import__('core')");
    // PyRun_SimpleString("import unicodedata"); // inject unicodedata into sys.modules
}

int main(int argc, char* argv[])
{    
    if (niraicall_onPreStart(argc, argv))
        return 1;
    
    Py_NoSiteFlag++;
    Py_FrozenFlag++;
    Py_InspectFlag++; // for crash at the exit
    
    Py_SetProgramName(argv[0]);
    Py_Initialize();
    PyEval_InitThreads();
    PySys_SetArgv(argc, argv);
    
    start_nirai();
    setup_python();
    
    if (niraicall_onLoadGameData())
        return 2;

    PyObject* res = PyImport_ImportModule("NiraiStart");
     
    if (res == NULL)
    {
        std::cerr << "Error importing NiraiStart!" << std::endl;
        PyErr_Print();
            
        return 3;
    }
    
    return 0;
}
