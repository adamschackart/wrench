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
- Multiple userdata slots for quick user library handle retrieval.
- Optional standard library modules for file I/O, directory enumeration, images, metaprogramming, etc.

# TODO

- Documentation system.
- Testing system.
- Hot reloading.
- Wrench++ - compatibility layer with Wren++ or wrenbind17.
- Compatibility with wren-cli.
- Copy path resolution/relative import logic from wren-cli.
- Visual Studio solution/project files.
- read-eval-print loop (repl).
- Debugger.
- Static analyzer.

- File-like object that writes to memory.
- Thread and thread pool objects.

- regular expressions (regex) module.
- sqlite module.
- date and time module.
- json module.
- xml module.
- diff module.
- base64 module.
- csv module.
- logging module.
- curses module.

- openssl wrapper.
- libuv wrapper.
- zeromq wrapper.
- sdl wrapper.
- opengl wrapper.
- openal wrapper.
- libffi wrapper.
