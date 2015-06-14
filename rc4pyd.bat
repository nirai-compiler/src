rem Compiles rc4.cxx to a pyd

@echo off
cl /nologo /EHsc /DRC4_BUILD_PYD /I../python/include rc4.cxx /link /LIBPATH:"C:\\Python27\\libs" /OUT:rc4.pyd /DLL
del rc4.obj
del rc4.lib
del rc4.exp
