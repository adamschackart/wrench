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

# Getting Started

Linux/MacOS users can type `sh build.sh` to bootstrap, then `./run_wren project build debug builtin-stdlib` to patch in a few additional features.
Windows users must use a developer terminal session to run the command at the top of `build.sh`, or run the experimental `build.bat`.
If git is not installed, external dependencies must be copied into the `wrench` directory.

# TODO

- Documentation system.
- Testing system.
- Hot reloading.
- Wrench++ - compatibility layer with Wren++ or wrenbind17.
- Compatibility with wren-cli.
- Copy path resolution/relative import logic from wren-cli.
- read-eval-print loop (repl).
- Debugger.
- Static analyzer.
- Tree shaker.
- Linter.

- File-like object that writes to memory.
- Thread and thread pool objects.

- regular expressions (regex) module.
- json module.
- xml module.
- diff module.
- base64 module.
- csv module.
- logging module.

- sqlite wrapper.
- curses wrapper.
- openssl wrapper.
- libuv wrapper.
- zeromq wrapper.
- sdl wrapper.
- opengl wrapper.
- openal wrapper.
- libffi wrapper.
