@echo off
setlocal enabledelayedexpansion
set cflags=
if "%1" == "-g" (
    shift
    set cflags=-D WAH_DEBUG -g
)
set run=0
for %%i in (wah_test_%1*.c) do (
    echo ## Running %%i...
    clang -W -Wall -Wextra -march=native %cflags% %%i -o %%~ni && %%~ni
    if !errorlevel! neq 0 (
        echo.
        echo ## %%i failed.
        goto end
    )
    echo.
    set run=1
)
if "%run%" == "1" (
    echo ## All tests passed.
) else (
    echo ## No tests run.
)
:end
