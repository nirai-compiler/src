# Nirai Compiler - src
This is the core repository of the Nirai compiler, which contains the actual compiler source code.

# What is Nirai?
Nirai is a game deployment tool that is designed for use on [Panda3D](https://www.panda3d.org/) projects.

Nirai's purpose is to make it easier for Panda3D developers to get their projects out to the public, as well as preventing source code dumps and arbitrary Python code injection, which is possible with built-in Panda3D utilities such as "ppackage".

Nirai is meant to be used for compiling the source code of Panda3D games for production. A static executable is made, which by default contains Panda3D and Python, however you may link any other .pyd/.dll files to this executable if you wish (ex. [libpandadna](https://github.com/loblao/libpandadna)).

A sample Nirai project can be found [here](https://github.com/nirai-compiler/sample-project)

Nirai is currently supported on Windows, Mac OS X, and Linux.

# Usage

The following instructions will explain how to compile the Nirai [Sample Project](https://github.com/nirai-compiler/sample-project).

## Before we begin

Download the following projects linked below and place them in the expected folder structure. If you do not have a folder named *Nirai*, make one somewhere on your computer.

Expected folder structure:

> Nirai/

> [Nirai/panda3d/](https://github.com/nirai-compiler/panda3d)

> [Nirai/panda3d/thirdparty/](https://github.com/nirai-compiler/thirdparty)

> [Nirai/src/](https://github.com/nirai-compiler/src/)

> [Nirai/sample-project/](https://github.com/nirai-compiler/sample-project)

> [Nirai/python/](https://github.com/nirai-compiler/python/)

You will need to have *CMake* in order to build *Nirai's Python*. You can obtain a copy of *CMake* via your local OS' package manager (such as *Brew*), or obtain it directly from their website [here](https://cmake.org/download/).

You will also need to change the following commands from `/path/to/*` to your own local computer's path.

## Windows:

```
cd path/to/Nirai/panda3d/
compile.bat
postbuild.bat
cd path/to/src/
aespyd.bat
```

Open up `Nirai/python` and run CMake, then build the project CMake generates.

```
cd path/to/Nirai/sample-project/build/
python -OO make.py -cnm
```

## Mac/Linux

```
cd path/to/Nirai/panda3d/
sh ./compile.sh
sh ./postbuild.sh
cd ../src/
sh ./aespyd.sh
cd ../python/
cmake . && make
cd ../sample-project/build/
python -OO make.py -cnm
```

If you have a conflicting version of *OpenSSL* installed, you will get a linker error similar to "*invalid architecture for symbols x86*".
