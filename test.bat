@echo off
setlocal enabledelayedexpansion
set cflags=-D WAH_ASSERT=assert
if "%1" == "-g" (
    shift
    set cflags=-D WAH_DEBUG -g
)
set run=0
for %%i in (tests\test_%1*.c) do (
    echo ## Running %%~nxi...
    clang -W -Wall -Wextra %cflags% %%i -o tests\%%~ni.exe && tests\%%~ni.exe && del tests\%%~ni.exe
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
