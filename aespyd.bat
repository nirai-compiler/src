@echo off

rem Compiles aes.cxx to a .pyd file.

set CMD=cl /c /nologo /EHsc /DBUILD_PYD /I../panda3d/thirdparty/win-python/include
set CMD=%CMD% /I../panda3d/thirdparty/win-libs-vc10/openssl/include aes.cxx
%CMD%

set CMD=link /DLL /LIBPATH:"../panda3d/thirdparty/win-python/libs"
set CMD=%CMD% /LIBPATH:"../panda3d/thirdparty/win-libs-vc10/openssl/lib" /OUT:aes.pyd aes.obj libpandaeay.lib
set CMD=%CMD% advapi32.lib gdi32.lib user32.lib crypt32.lib
%CMD%

del aes.obj
del aes.lib
del aes.exp
