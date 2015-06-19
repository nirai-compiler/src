# Nirai Compiler - src
This is the core repository of the Nirai compiler, which contains the actual compiler source code.

# What is Nirai?
Nirai is a game deployment tool that is designed for use on [Panda3D](https://www.panda3d.org/) projects.

Nirai's purpose is to make it easier for Panda3D developers to get their projects out to the public, as well as preventing source code dumps and arbitrary Python code injection, which is possible with built-in Panda3D utilities such as "ppackage".

Nirai is meant to be used for compiling the source code of Panda3D games for production. A static executable is made, which by default contains Panda3D and Python, however you may link any other .pyd/.dll files to this executable if you wish (ex. [libpandadna](https://github.com/loblao/libpandadna)).

A sample Nirai project can be found [here](https://github.com/nirai-compiler/sample-project)

Currently, Nirai has been tested only on Windows. Support for Linux and Mac is planned.
