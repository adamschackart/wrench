@echo off
setlocal enabledelayedexpansion

:: -----------------------------------------------------------------------------
:: Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
:: Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
:: -----------------------------------------------------------------------------

if not exist "stb\" (
    if exist "..\extern\stb\" (
        xcopy /E /I /Q "..\extern\stb" "stb"
    ) else (
        git clone https://github.com/nothings/stb.git
    )
)

if not exist "subprocess.h\" (
    if exist "..\extern\subprocess.h\" (
        xcopy /E /I /Q "..\extern\subprocess.h" "subprocess.h"
    ) else (
        git clone https://github.com/sheredom/subprocess.h.git
    )
)

if not exist "tinydir\" (
    if exist "..\extern\tinydir\" (
        xcopy /E /I /Q "..\extern\tinydir" "tinydir"
    ) else (
        git clone https://github.com/cxong/tinydir.git
    )
)

if not exist "wren\" (
    if exist "..\extern\wren\" (
        xcopy /E /I /Q "..\extern\wren" "wren"
    ) else (
        git clone https://github.com/wren-lang/wren.git
    )

    python3 -B wren/util/generate_amalgamation.py >> wren.c
)

if not exist "zip\" (
    if exist "..\extern\zip\" (
        xcopy /E /I /Q "..\extern\zip" "zip"
    ) else (
        git clone https://github.com/kuba--/zip.git
    )
)

:: set COMPILER_FLAGS=/Ot /Ox /DNDEBUG
set COMPILER_FLAGS=/Od /Zi /DEBUG

set BUILTIN_STDLIB=true

:: XXX: "run_wren.exe" can't generate a file with the same name due to locking, so we build "wrench_main.exe".
if "%BUILTIN_STDLIB%"=="true" (
    cl %COMPILER_FLAGS% /nologo /I. /Iwren/src/include /DWRENCH_STDLIB=1 /Fe:wrench_main.exe wrench_main.c wren.c
) else (
    cl %COMPILER_FLAGS% /nologo /I. /Iwren/src/include /c wren.c /Fo:wren.obj

    cl %COMPILER_FLAGS% /nologo /I. /Iwren/src/include /Fe:wrench_main.exe wrench_main.c wren.obj
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
