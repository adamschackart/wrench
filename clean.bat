@echo off
setlocal enabledelayedexpansion

:: -----------------------------------------------------------------------------
:: Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
:: Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
:: -----------------------------------------------------------------------------

rmdir /s /q stb
rmdir /s /q subprocess.h
rmdir /s /q tinydir
rmdir /s /q wren
rmdir /s /q zip

del wren.c /q
del *.exe /q
del *.lib /q
del *.obj /q
del *.dll /q
del *.exp /q

del wrench_config.c /q
del wrench_project.c /q
