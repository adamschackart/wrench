# ------------------------------------------------------------------------------
# Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
# Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
# ------------------------------------------------------------------------------
# Easy Windows build in Developer Powershell for VS 2022 (must have libraries):
# cl /nologo /Od /DEBUG /DWRENCH_STDLIB=1 /I. /Iwren\src\include /Iwren\src\optional /Iwren\src\vm wrench_main.c wren\src\optional\wren_opt_meta.c wren\src\optional\wren_opt_random.c wren\src\vm\wren_compiler.c wren\src\vm\wren_core.c wren\src\vm\wren_debug.c wren\src\vm\wren_primitive.c wren\src\vm\wren_utils.c wren\src\vm\wren_value.c wren\src\vm\wren_vm.c
# ------------------------------------------------------------------------------

if [ ! -d "stb" ]; then
    if [ -d "../extern/stb" ]; then
        cp -r ../extern/stb stb
    else
        git clone https://github.com/nothings/stb.git
    fi
fi

if [ ! -d "subprocess.h" ]; then
    if [ -d "../extern/subprocess.h" ]; then
        cp -r ../extern/subprocess.h subprocess.h
    else
        git clone https://github.com/sheredom/subprocess.h.git
    fi
fi

if [ ! -d "tinydir" ]; then
    if [ -d "../extern/tinydir" ]; then
        cp -r ../extern/tinydir tinydir
    else
        git clone https://github.com/cxong/tinydir.git
    fi
fi

if [ ! -d "wren" ]; then
    if [ -d "../extern/wren" ]; then
        cp -r ../extern/wren wren
    else
        git clone https://github.com/wren-lang/wren.git
    fi

    python3 -B wren/util/generate_amalgamation.py >> wren.c
fi

if [ ! -d "zip" ]; then
    if [ -d "../extern/zip" ]; then
        cp -r ../extern/zip zip
    else
        git clone https://github.com/kuba--/zip.git
    fi
fi

# TODO: Optional command-line argument to choose compiler - use export for now.
if false; then
    COMPILER=clang
elif false; then
    COMPILER=gcc
elif false; then
    COMPILER=tcc
else
    COMPILER=cc
fi

# TODO: Optional command-line argument to enable release build or -Os (smaller).
if true; then
    COMPILER_FLAGS="-O0 -g"
else
    COMPILER_FLAGS="-O3"
fi

# TODO: Optional command-line argument to enable unity build / stdlib inclusion.
BUILTIN_STDLIB=true

if "$BUILTIN_STDLIB"; then
    $COMPILER $COMPILER_FLAGS -DWRENCH_STDLIB=1 -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -o run_wren wrench_main.c wren.c -lm -ldl
else
    $COMPILER $COMPILER_FLAGS -fPIC -c -o wren.o wren.c

    $COMPILER $COMPILER_FLAGS -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -o run_wren wrench_main.c wren.o -lm -ldl &
    $COMPILER $COMPILER_FLAGS -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -o file.so wrench_file.c wren.o -lm -ldl &
    $COMPILER $COMPILER_FLAGS -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -o image.so wrench_image.c wren.o -lm -ldl &
    $COMPILER $COMPILER_FLAGS -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -o platform.so wrench_platform.c wren.o -lm -ldl &
    $COMPILER $COMPILER_FLAGS -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -o process.so wrench_process.c wren.o -lm -ldl &
    $COMPILER $COMPILER_FLAGS -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -o rect.so wrench_rect.c wren.o -lm -ldl &
    $COMPILER $COMPILER_FLAGS -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -o time.so wrench_time.c wren.o -lm -ldl &
    $COMPILER $COMPILER_FLAGS -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -o util.so wrench_util.c wren.o -lm -ldl &
    $COMPILER $COMPILER_FLAGS -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -o vector.so wrench_vector.c wren.o -lm -ldl &
    $COMPILER $COMPILER_FLAGS -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -o vm.so wrench_vm.c wren.o -lm -ldl &
    $COMPILER $COMPILER_FLAGS -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -o zip.so wrench_zip.c wren.o -lm -ldl &

    #$COMPILER $COMPILER_FLAGS -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -ltcc -o tcc.so wrench_tcc.c wren.o -lm -ldl &

    wait
fi
