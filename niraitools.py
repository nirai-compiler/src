assert not __debug__ # Run with -OO

from panda3d.core import *

from collections import OrderedDict
import subprocess, glob, sys, os

import niraimarshal
import aes

SOURCE_ROOT = os.path.dirname(os.path.abspath(__file__))
NIRAI_ROOT = os.path.abspath(os.path.join(SOURCE_ROOT, '..'))
PYTHON_ROOT = os.path.join(NIRAI_ROOT, 'python')
PANDA3D_ROOT = os.path.join(NIRAI_ROOT, 'panda3d')
THIRDPARTY_ROOT = os.path.join(PANDA3D_ROOT, 'thirdparty')

class NiraiCompiler:
    def __init__(self, output, outputdir='built',
                 includedirs=set(), libs=set(), libpath=set()):
        self.output = output
        self.outputdir = outputdir
        self.thirdpartydir = os.path.join(THIRDPARTY_ROOT, 'win-libs-vc10')

        self.includedirs = includedirs.copy()
        self.includedirs.add(os.path.join(PANDA3D_ROOT, 'built', 'include'))
        self.includedirs.add(os.path.join(PYTHON_ROOT, 'Include'))
        self.includedirs.add(SOURCE_ROOT)

        self.libs = libs.copy()

        self.libpath = libpath.copy()

        self.builtLibs = os.path.join(NIRAI_ROOT, 'panda3d', 'built', 'lib')
        self.libpath.add(self.builtLibs)
        self.libpath.add(os.path.join(NIRAI_ROOT, 'python'))

        self.sources = set()
        self.__built = set()

    def add_source(self, filename):
        self.sources.add(filename)

    def add_library(self, lib, thirdparty=False):
        if thirdparty:
            root = os.path.normpath(lib).split(os.sep)[0]
            self.includedirs.add(os.path.join(self.thirdpartydir, root, 'include'))
            
            lib = os.path.join(self.thirdpartydir, lib)

        self.libs.add(lib + '.lib')

    def add_nirai_files(self):
        for filename in ('aes.cxx', 'main.cxx'):
            self.add_source(os.path.join(SOURCE_ROOT, filename))

        self.libs |= set(glob.glob(os.path.join(self.builtLibs, '*.lib')))

        self.add_library('pythonembed')

        self.add_library('ws2_32')
        self.add_library('shell32')
        self.add_library('advapi32')
        self.add_library('gdi32')
        self.add_library('user32')
        self.add_library('oleaut32')
        self.add_library('ole32')
        self.add_library('iphlpapi')
        self.add_library('shell32')
        self.add_library('wsock32')
        self.add_library('opengl32')
        self.add_library('imm32')
        self.add_library('crypt32')

        self.add_library('nvidiacg\\lib\\cgGL', thirdparty=True)
        self.add_library('nvidiacg\\lib\\cgD3D9', thirdparty=True)
        self.add_library('nvidiacg\\lib\\cg', thirdparty=True)
        self.add_library('squish\\lib\\squish', thirdparty=True)
        self.add_library('freetype\\lib\\freetype', thirdparty=True)
        self.add_library('vorbis\\lib\\libogg_static', thirdparty=True)
        self.add_library('vorbis\\lib\\libvorbis_static', thirdparty=True)
        self.add_library('vorbis\\lib\\libvorbisfile_static', thirdparty=True)
        self.add_library('ode\\lib\\ode_single', thirdparty=True)
        self.add_library('openal\\lib\\OpenAL32', thirdparty=True)
        self.add_library('openssl\\lib\\libpandaeay', thirdparty=True)
        self.add_library('openssl\\lib\\libpandassl', thirdparty=True)
        self.add_library('png\\lib\\libpng_static', thirdparty=True)
        self.add_library('jpeg\\lib\\jpeg-static', thirdparty=True)
        self.add_library('tiff\\lib\\libtiff', thirdparty=True)
        self.add_library('fftw\\lib\\fftw', thirdparty=True)
        self.add_library('fftw\\lib\\rfftw', thirdparty=True)
        self.add_library('zlib\\lib\\zlibstatic', thirdparty=True)

    def __run_command(self, cmd):
        p = subprocess.Popen(cmd, stdout=sys.stdout, stderr=sys.stderr)
        v = p.wait()

        if v != 0:
            print 'The following command returned non-zero value (%d): %s' % (v, cmd[:100] + '...')
            sys.exit(1)

    def __compile(self, filename):
        out = '%s/%s.obj' % (self.outputdir, os.path.basename(filename).rsplit('.', 1)[0])

        cmd = 'cl /c /GF /MP4 /DPy_BUILD_CORE /DNTDDI_VERSION=0x0501 /wd4996 /wd4275 /wd4267 /wd4101 /wd4273 /nologo /EHsc /MD /Zi /O2'
        for ic in self.includedirs:
            cmd += ' /I"%s"' % ic

        cmd += ' /Fo%s "%s"' % (out, filename)

        self.__run_command(cmd)
        self.__built.add(out)

    def __link(self):
        cmd = 'link /LTCG /LTCG:STATUS /nologo /out:%s/%s' % (self.outputdir, self.output)
        for obj in self.__built:
            cmd += ' "%s"' % obj

        for lib in self.libs:
            cmd += ' "%s"' % lib

        for path in self.libpath:
            cmd += ' /LIBPATH:"%s"' % path

        cmd += ' /RELEASE /nodefaultlib:python27.lib /nodefaultlib:libcmt /ignore:4049 /ignore:4006 /ignore:4221'
        self.__run_command(cmd)

    def run(self):
        print 'Compiling CXX codes...'
        for filename in self.sources:
            self.__compile(filename)

        print 'Linking...'
        self.__link()

class NiraiPackager:
    HEADER = 'NRI\n'

    def __init__(self, outfile):
        self.modules = OrderedDict()
        self.outfile = outfile

    def __read_file(self, filename, mangler=None):
        with open(filename, 'rb') as f:
            data = f.read()

        base = filename.rsplit('.', 1)[0].replace('\\', '/').replace('/', '.')
        pkg = base.endswith('.__init__')
        moduleName = base.rsplit('.', 1)[0] if pkg else base

        name = moduleName
        if mangler is not None:
            name = mangler(name)

        if not name:
            return '', ('', 0)

        try:
            data = self.compile_module(name, data)

        except:
            print 'WARNING: Failed to compile', filename
            return '', ('', 0)

        size = len(data) * (-1 if pkg else 1)
        return name, (data, size)

    def compile_module(self, name, data):
        return niraimarshal.dumps(compile(data, name, 'exec'))

    def add_module(self, moduleName, data, size=None, compile=False, negSize=False):                
        if compile:
            data = self.compile_module(moduleName, data)

        if size is None:
            size = len(data)
            if negSize:
                size = -size

        self.modules[moduleName] = (data, size)

    def add_file(self, filename, mangler=None):
        print 'Adding file', filename
        moduleName, (data, size) = self.__read_file(filename, mangler)
        if moduleName:
            moduleName = os.path.basename(filename).rsplit('.', 1)[0]
            self.add_module(moduleName, data, size)

    def add_directory(self, dir, mangler=None):
        print 'Adding directory', dir

        def _recurse_dir(dir):
            for f in os.listdir(dir):
                f = os.path.join(dir, f)

                if os.path.isdir(f):
                    _recurse_dir(f)

                elif f.endswith('py'):
                    moduleName, (data, size) = self.__read_file(f, mangler)
                    if moduleName:
                        self.add_module(moduleName, data, size)

        _recurse_dir(dir)

    def get_mangle_base(self, *path):
        return len(os.path.join(*path).rsplit('.', 1)[0].replace('\\', '/').replace('/', '.')) + 1

    def add_panda3d_dirs(self):
        manglebase = self.get_mangle_base(NIRAI_ROOT, 'panda3d', 'built')

        def _mangler(name):
            name = name[manglebase:].strip('.')
            
            # Required hack
            if name == 'direct.extensions_native.extension_native_helpers':
                name = 'extension_native_helpers'
            
            return name

        self.add_directory(os.path.join(NIRAI_ROOT, 'panda3d', 'built', 'direct'), mangler=_mangler)
        self.add_directory(os.path.join(NIRAI_ROOT, 'panda3d', 'built', 'pandac'), mangler=_mangler)
        self.add_directory(os.path.join(NIRAI_ROOT, 'panda3d', 'built', 'panda3d'), mangler=_mangler)

    def add_default_lib(self):
        manglebase = self.get_mangle_base(NIRAI_ROOT, 'python', 'Lib')

        def _mangler(name):
            name = name[manglebase:]
            return name.strip('.')

        self.add_directory(os.path.join(NIRAI_ROOT, 'python', 'Lib'), mangler=_mangler)

    def write_out(self):
        f = open(self.outfile, 'wb')
        f.write(self.HEADER)
        f.write(self.process_modules())
        f.close()

    def generate_key(self, size=256):
        return os.urandom(size)

    def dump_key(self, key):
        for k in key:
            print ord(k),

        print

    def process_modules(self):
        # Pure virtual
        raise NotImplementedError('process_datagram')

    def get_file_contents(self, filename, encrypt=False):
        with open(filename, 'rb') as f:
            data = f.read()
            
        if encrypt:
            iv = self.generate_key(16)
            key = self.generate_key(16)
            data = iv + key + aes.encrypt(data, key, iv)

        return data
