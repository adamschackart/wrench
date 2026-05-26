# Wrench

`wrench.h` is a single-header-file library containing a complete [Wren](http://github.com/wren-lang/wren/) programming environment.
While vanilla Wren leaves many implementation details up to the user, Wrench fills in those gaps to provide a standard, modular framework designed to get things done quickly and facilitate code sharing.
By using the extended VM, you get access to:
- Customizable loading of Wren scripts.
- Building scripts incrementally within C code.
- Retrieval of all loaded script names and their source code.
- Automatic shared library loading for foreign methods and classes.
- Disabling of native code loading for security.
- An entry point (main function) for easily running Wren scripts or foreign modules.
- Easy retrieval of command-line arguments.
- More slot types (int, size_t, single-precision float, etc).
- Multiple userdata slots for quick user library handle retrieval without global state.
- Optional standard library modules for file I/O, directory enumeration, images, metaprogramming, etc.
- A build system written in Wren that can be used to build Wrench itself.

# Getting Started (a Wren-first approach)

Linux/MacOS users can type `sh build.sh` to bootstrap, then `./run_wren project build debug builtin-stdlib` to patch in a few additional features and optimizations to the vanilla core Wren VM.
Windows users follow roughly the same process via `.\build.bat` and `.\wrench_main.exe project build debug builtin-stdlib` in a developer terminal session.
If git is not installed, external standard library dependencies must be copied into either the `wrench` or `../extern` directory.

This will give you an executable that can run arbitrary scripts or native libraries, with every Wrench standard library module already built-in.
<br>These scripts can have optional `var main = Fn.new { ... }` functions, so libraries can ship with small test suites or utilities.

# Getting Started (a C-first approach)

If you already have a native codebase you want to extend with Wren scripting, simply add the following C code to your project:

```c
#include <wren.c>

#define WRENCH_IMPLEMENTATION 1
#include <wrench/wrench.h>
```

An excellent example of bridging C with wren is available in `wrench_image.c` or `wrench_file.c`, starting at the bottom of the file and going upwards.

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

# TODO (stdlib modules)

- regular expressions (regex) module.
- json module.
- xml module.
- diff module.
- base64 module.
- csv module.
- logging module.

# TODO (external library wrappers)

- sqlite wrapper.
- curses wrapper.
- openssl wrapper.
- libuv wrapper.
- zeromq wrapper.
- sdl wrapper (with game_main executable that runs the SDL3 application loop/callbacks).
- opengl wrapper.
- openal wrapper.
- libffi wrapper.
