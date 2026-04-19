@echo off
setlocal enabledelayedexpansion
set cflags=-D WAH_ASSERT=assert
if "%1" == "-g" (
    shift
    set cflags=-D WAH_DEBUG -g
)

set pattern=tests\test_%1*.c
set exclude_spectest=0
if "%~1" == "" (
    set pattern=tests\test_*.c
    set exclude_spectest=1
)
if /I "%~1" == "spec" (
    set pattern=tests\test_spec*.c
    set exclude_spectest=1
)
if /I "%~1" == "spectest" (
    set pattern=tests\test_spectest.c
    set exclude_spectest=0
)

set run=0
for %%i in (%pattern%) do (
    if "!exclude_spectest!" == "1" (
        if /I not "%%~ni" == "test_spectest" (
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
    ) else (
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
)
if "%run%" == "1" (
    echo ## All tests passed.
) else (
    echo ## No tests run.
)
:end
