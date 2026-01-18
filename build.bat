@echo off
setlocal enabledelayedexpansion

:: -----------------------------------------------------------------------------
:: Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
:: Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
:: -----------------------------------------------------------------------------

if not exist "stb\" (
    git clone https://github.com/nothings/stb.git
)

if not exist "subprocess.h\" (
    git clone https://github.com/sheredom/subprocess.h.git
)

if not exist "tinydir\" (
    git clone https://github.com/cxong/tinydir.git
)

if not exist "wren\" (
    git clone https://github.com/wren-lang/wren.git
    python3 -B wren/util/generate_amalgamation.py >> wren.c
)

if not exist "zip\" (
    git clone https://github.com/kuba--/zip.git
)

:: set COMPILER_FLAGS=/Ot /Ox /DNDEBUG
set COMPILER_FLAGS=/Od /Zi /DEBUG

set BUILTIN_STDLIB=true

if "%BUILTIN_STDLIB%"=="true" (
    cl %COMPILER_FLAGS% /nologo /I. /Iwren/src/include /DWRENCH_STDLIB=1 /Fe:run_wren.exe wrench_main.c wren.c
) else (
    cl %COMPILER_FLAGS% /nologo /I. /Iwren/src/include /c wren.c /Fo:wren.obj

    cl %COMPILER_FLAGS% /nologo /I. /Iwren/src/include /Fe:run_wren.exe wrench_main.c wren.obj
    cl %COMPILER_FLAGS% /nologo /I. /Iwren/src/include /LD /Fe:file.dll wrench_file.c wren.obj
    cl %COMPILER_FLAGS% /nologo /I. /Iwren/src/include /LD /Fe:image.dll wrench_image.c wren.obj
    cl %COMPILER_FLAGS% /nologo /I. /Iwren/src/include /LD /Fe:platform.dll wrench_platform.c wren.obj
    cl %COMPILER_FLAGS% /nologo /I. /Iwren/src/include /LD /Fe:process.dll wrench_process.c wren.obj
    cl %COMPILER_FLAGS% /nologo /I. /Iwren/src/include /LD /Fe:rect.dll wrench_rect.c wren.obj
    cl %COMPILER_FLAGS% /nologo /I. /Iwren/src/include /LD /Fe:time.dll wrench_time.c wren.obj
    cl %COMPILER_FLAGS% /nologo /I. /Iwren/src/include /LD /Fe:util.dll wrench_util.c wren.obj
    cl %COMPILER_FLAGS% /nologo /I. /Iwren/src/include /LD /Fe:vector.dll wrench_vector.c wren.obj
    cl %COMPILER_FLAGS% /nologo /I. /Iwren/src/include /LD /Fe:vm.dll wrench_vm.c wren.obj
    cl %COMPILER_FLAGS% /nologo /I. /Iwren/src/include /LD /Fe:zip.dll wrench_zip.c wren.obj
)
