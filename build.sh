# ------------------------------------------------------------------------------
# Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
# Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
# ------------------------------------------------------------------------------

if [ ! -d "stb" ]; then
    git clone https://github.com/nothings/stb.git
fi

if [ ! -d "subprocess.h" ]; then
    git clone https://github.com/sheredom/subprocess.h.git
fi

if [ ! -d "tinydir" ]; then
    git clone https://github.com/cxong/tinydir.git
fi

if [ ! -d "wren" ]; then
    git clone https://github.com/wren-lang/wren.git
    python3 -B wren/util/generate_amalgamation.py >> wren.c
fi

if [ ! -d "zip" ]; then
    git clone https://github.com/kuba--/zip.git
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
BUILTIN_STDLIB=false

if "$BUILTIN_STDLIB"; then
    $COMPILER $COMPILER_FLAGS -DWRENCH_STDLIB=1 -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -o run_wren wrench_main.c wren.c -lm -ldl
else
    $COMPILER $COMPILER_FLAGS -fPIC -c -o wren.o wren.c

    $COMPILER $COMPILER_FLAGS -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -o run_wren wrench_main.c wren.o -lm -ldl &
    $COMPILER $COMPILER_FLAGS -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -o file.so wrench_file.c wren.o -lm -ldl &
    $COMPILER $COMPILER_FLAGS -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -o image.so wrench_image.c wren.o -lm -ldl &
    $COMPILER $COMPILER_FLAGS -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -o process.so wrench_process.c wren.o -lm -ldl &
    $COMPILER $COMPILER_FLAGS -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -o rect.so wrench_rect.c wren.o -lm -ldl &
    $COMPILER $COMPILER_FLAGS -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -o util.so wrench_util.c wren.o -lm -ldl &
    $COMPILER $COMPILER_FLAGS -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -o vector.so wrench_vector.c wren.o -lm -ldl &
    $COMPILER $COMPILER_FLAGS -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -o vm.so wrench_vm.c wren.o -lm -ldl &
    $COMPILER $COMPILER_FLAGS -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -o zip.so wrench_zip.c wren.o -lm -ldl &

    #$COMPILER $COMPILER_FLAGS -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -ltcc -o tcc.so wrench_tcc.c wren.o -lm -ldl &

    wait
fi
