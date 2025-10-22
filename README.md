# Wrench

`wrench.h` is a single-file library containing a complete [Wren](http://github.com/wren-lang/wren/) programming environment. While vanilla Wren leaves many implementation details up to the user, Wrench fills in those gaps to provide a standard, modular framework designed to get things done quickly and facilitate code sharing. By using the extended VM, you get access to:
- Customizable loading of Wren scripts.
- Building scripts incrementally within C code.
- Retrieval of all loaded script names and their source code.
- Automatic shared library loading for foreign methods and classes.
- Disabling of native code loading for security.
- An entry point (main function) for easily running Wren scripts or foreign modules.
- Easy retrieval of command-line arguments.
- More slot types.
- Multiple userdata slots for quick library handle retrieval.
- Optional standard library modules for file I/O, directory enumeration, etc.

# TODO

- Hot reloading.
- Wrench++ - compatibility layer with Wren++.
- Better build system.
