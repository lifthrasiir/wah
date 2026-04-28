param(
    [switch]$g,
    [switch]$clang,
    [switch]$gcc,
    [switch]$msvc,
    [Parameter(ValueFromRemainingArguments)]
    [string[]]$Remaining
)

$ErrorActionPreference = 'Stop'

# --- Compiler selection ---
$compilerCount = ([int]$clang.IsPresent + [int]$gcc.IsPresent + [int]$msvc.IsPresent)
if ($compilerCount -gt 1) {
    Write-Host '## Error: specify at most one of -clang, -gcc, -msvc.'
    exit 1
}
if ($msvc) {
    $compiler = 'msvc'
} elseif ($gcc) {
    $compiler = 'gcc'
} else {
    $compiler = 'clang'
}

if ($compiler -eq 'clang') {
    $clangCandidates = @(
        'C:\msys64\clang64\bin\clang.exe',
        'C:\msys64\ucrt64\bin\clang.exe'
    )
    $msys2Clang = $clangCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
    if ($msys2Clang) {
        $compiler = $msys2Clang
    }
} elseif ($compiler -eq 'gcc') {
    $msys2Gcc = 'C:\msys64\ucrt64\bin\gcc.exe'
    if (Test-Path $msys2Gcc) {
        $compiler = $msys2Gcc
    }
}

$compilerKind = if ($msvc) { 'msvc' } elseif ($gcc) { 'gcc' } else { 'clang' }
Write-Host "## Using $compilerKind compiler: $compiler"

$filter = ''
if ($Remaining.Count -gt 0) { $filter = $Remaining[0] }

$projDir = $PSScriptRoot
if (-not $projDir) { $projDir = Get-Location }

# --- MSVC environment setup ---
if ($compilerKind -eq 'msvc') {
    $vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vsWhere)) {
        Write-Host "## Error: vswhere.exe not found at $vsWhere"
        Write-Host '## Make sure Visual Studio is installed.'
        exit 1
    }
    $vsPath = & $vsWhere -latest -version '[16.8,' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (-not $vsPath) {
        Write-Host '## Error: Visual Studio 2019 16.8+ with C++ tools is required (for /std:c11 support).'
        exit 1
    }
    $vcvarsall = "$vsPath\VC\Auxiliary\Build\vcvarsall.bat"
    if (-not (Test-Path $vcvarsall)) {
        Write-Host "## Error: vcvarsall.bat not found at $vcvarsall"
        exit 1
    }
    # Import MSVC environment variables into this PowerShell session.
    $envBefore = @{}
    Get-ChildItem env: | ForEach-Object { $envBefore[$_.Name] = $_.Value }
    $output = cmd /c "`"$vcvarsall`" x64 >nul 2>&1 && set"
    foreach ($line in $output) {
        if ($line -match '^([^=]+)=(.*)$') {
            $n = $matches[1]; $v = $matches[2]
            if ($envBefore[$n] -ne $v) {
                Set-Item "env:$n" $v
            }
        }
    }
    Write-Host "## Using MSVC from $vsPath"
}

# --- Compiler driver helpers ---
$objExt = if ($compilerKind -eq 'msvc') { '.obj' } else { '.o' }

# --- Bench command ---
if ($filter -eq 'bench') {
    $benchSrc = "$projDir\bench\bench_coremark.c"
    $benchExe = "$projDir\bench\bench_coremark.exe"
    if ($compilerKind -eq 'msvc') {
        $benchCflags = @('/W4', '/O2', '/DWAH_ASSERT=assert')
    } else {
        $benchCflags = @('-W', '-Wall', '-Wextra', '-DWAH_ASSERT=assert', '-O3')
    }
    Write-Host "## Compiling bench_coremark ($compilerKind)..."
    if ($compilerKind -eq 'msvc') {
        & cl /nologo @benchCflags $benchSrc /Fe"$benchExe"
        $compExit = $LASTEXITCODE
        Remove-Item ($benchSrc -replace '\.c$', '.obj') -ErrorAction SilentlyContinue
    } else {
        & $compiler @benchCflags $benchSrc -o $benchExe
        $compExit = $LASTEXITCODE
    }
    if ($compExit -ne 0) {
        Write-Host '## Bench compilation failed.'
        exit 1
    }
    Write-Host '## Running CoreMark benchmark...'
    & $benchExe
    $benchExit = $LASTEXITCODE
    Remove-Item $benchExe -ErrorAction SilentlyContinue
    exit $benchExit
}

if ($compilerKind -eq 'msvc') {
    $cflags = @('/W4', '/std:c11', '/wd5105', '/D_CRT_SECURE_NO_WARNINGS')
    if ($g) {
        $cflags += '/DWAH_DEBUG', '/Zi'
    } else {
        $cflags += '/DWAH_ASSERT=assert'
    }
} else {
    $cflags = @('-W', '-Wall', '-Wextra')
    if ($g) {
        $cflags += '-DWAH_DEBUG', '-g'
    } else {
        $cflags += '-DWAH_ASSERT=assert'
    }
}

$testFiles = Get-ChildItem "$projDir\tests\test_$filter*.c" | Sort-Object Name
if ($testFiles.Count -eq 0) {
    Write-Host '## No tests found.'
    exit 1
}

# Classify: does the .c file define WAH_IMPLEMENTATION itself?
# If yes -> standalone: test.o embeds wah implementation, link with common.o
# If no  -> api-only: link with precompiled wah.o + common.o
function Test-NeedsImpl($file) {
    $reader = [System.IO.StreamReader]::new($file.FullName)
    try {
        while ($null -ne ($line = $reader.ReadLine())) {
            if ($line -match '^\s*#\s*define\s+WAH_IMPLEMENTATION\b') { return $true }
        }
    } finally { $reader.Close() }
    return $false
}

$standaloneTests = @()
$apiTests = @()
foreach ($f in $testFiles) {
    if (Test-NeedsImpl $f) {
        $standaloneTests += $f
    } else {
        $apiTests += $f
    }
}

# --- Precompiled objects ---
$wahImplObj = "$projDir\tests\wah_impl$objExt"
$commonObj = "$projDir\tests\common$objExt"

function Test-ObjUpToDate($obj, [string[]]$deps) {
    if (-not (Test-Path $obj)) { return $false }
    $objTime = (Get-Item $obj).LastWriteTime
    foreach ($d in $deps) {
        if ((Get-Item $d).LastWriteTime -gt $objTime) { return $false }
    }
    return $true
}

$wahH = "$projDir\wah.h"
$commonSrc = "$projDir\tests\common.c"
$commonHdr = "$projDir\tests\common.h"

# Always need common.o (used by both standalone and api-only tests).
$commonObjOk = Test-ObjUpToDate $commonObj @($wahH, $commonSrc, $commonHdr)

# Need wah_impl.o only if there are api-only tests.
$wahImplObjOk = $true
$wahImplSrc = "$projDir\tests\wah_impl.c"
$wahImplHdr = "$projDir\tests\wah_impl.h"
if ($apiTests.Count -gt 0) {
    $wahImplObjOk = Test-ObjUpToDate $wahImplObj @($wahH, $wahImplSrc, $wahImplHdr)
}

# -O1 for wah_impl so that musttail dispatch compiles to actual tail calls;
# without it, -O0 musttail is ~15x slower than switch dispatch.
$wahImplCflags = if ($compilerKind -ne 'msvc') { $cflags + '-O1' } else { $cflags }

$jobs = @()
if (-not $wahImplObjOk) {
    Write-Host "## Compiling wah_impl$objExt..."
    $jobs += Start-Job -ScriptBlock {
        param($compiler, $compilerKind, $cflags, $src, $out, $dir)
        Set-Location $dir
        if ($compilerKind -eq 'msvc') {
            & cl /nologo @cflags /c $src /Fo"$out" 2>&1
        } else {
            & $compiler @cflags -c $src -o $out 2>&1
        }
        $LASTEXITCODE
    } -ArgumentList $compiler, $compilerKind, $wahImplCflags, $wahImplSrc, $wahImplObj, $projDir
}
if (-not $commonObjOk) {
    Write-Host "## Compiling common$objExt..."
    $jobs += Start-Job -ScriptBlock {
        param($compiler, $compilerKind, $cflags, $src, $out, $dir)
        Set-Location $dir
        if ($compilerKind -eq 'msvc') {
            & cl /nologo @cflags /c $src /Fo"$out" 2>&1
        } else {
            & $compiler @cflags -c $src -o $out 2>&1
        }
        $LASTEXITCODE
    } -ArgumentList $compiler, $compilerKind, $cflags, $commonSrc, $commonObj, $projDir
}

$objFailed = $false
foreach ($j in $jobs) {
    $output = @(Receive-Job $j -Wait)
    if ($j.State -eq 'Failed') {
        $jobErr = $j.ChildJobs[0].JobStateInfo.Reason
        if ($null -ne $jobErr) { Write-Host $jobErr }
        Write-Host "## Precompiled object build failed."
        $objFailed = $true
    } elseif ($output.Count -eq 0) {
        Write-Host "## Precompiled object build failed without an exit code."
        $objFailed = $true
    }
    else {
        $exitCode = $output[-1]
        $messages = if ($output.Count -gt 1) { $output[0..($output.Count - 2)] } else { @() }
        if ($messages.Count -gt 0) { $messages | ForEach-Object { Write-Host $_ } }
        if ($exitCode -ne 0) {
            Write-Host "## Precompiled object build failed with exit code $exitCode."
            $objFailed = $true
        }
    }
    Remove-Job $j
}
if ($objFailed) { exit 1 }

# --- Build the ordered test queue ---
# Standalone tests first (longer to compile), then API-only tests.
$testQueue = @()
foreach ($f in $standaloneTests) { $testQueue += @{ File = $f; Standalone = $true } }
foreach ($f in $apiTests)        { $testQueue += @{ File = $f; Standalone = $false } }

# --- Pipeline: compile test[i+1] while running test[i] ---

function Start-Compile($test) {
    $f = $test.File
    $name = $f.BaseName
    $exe = "$projDir\tests\$name.exe"

    # standalone: test.obj has WAH_IMPLEMENTATION, link with common.obj
    # api-only: test.obj has no WAH_IMPLEMENTATION, link with wah_impl.obj + common.obj
    $linkObjStr = if ($test.Standalone) { $commonObj } else { "$wahImplObj|$commonObj" }

    return Start-Job -ScriptBlock {
        param($cc, $cflags, $src, $exe, $linkObjStr, $dir, $objExt)
        Set-Location $dir
        $linkObjs = $linkObjStr -split '\|'
        $obj = $exe -replace '\.exe$', $objExt
        if ($cc -eq 'msvc') {
            & cl /nologo @cflags /c $src /Fo"$obj" 2>&1
            if ($LASTEXITCODE -ne 0) { $LASTEXITCODE; return }
            & cl /nologo $obj @linkObjs /Fe"$exe" /link /nologo 2>&1
        } else {
            & $cc @cflags -c $src -o $obj 2>&1
            if ($LASTEXITCODE -ne 0) { $LASTEXITCODE; return }
            & $cc $obj @linkObjs -o $exe 2>&1
        }
        $lec = $LASTEXITCODE
        Remove-Item $obj -ErrorAction SilentlyContinue
        $lec
    } -ArgumentList $script:compiler, $script:cflags, $f.FullName, $exe, $linkObjStr, $script:projDir, $script:objExt
}

function Wait-Compile($job, $test) {
    $output = @(Receive-Job $job -Wait)
    Remove-Job $job
    $exitCode = $output[-1]
    $messages = if ($output.Count -gt 1) { $output[0..($output.Count - 2)] } else { @() }
    if ($messages.Count -gt 0) { $messages | ForEach-Object { Write-Host $_ } }
    if ($exitCode -ne 0) {
        Write-Host "## Compilation of $($test.File.Name) failed."
        return $null
    }
    return "$script:projDir\tests\$($test.File.BaseName).exe"
}

$failed = $false
$ran = 0

if ($testQueue.Count -eq 0) {
    Write-Host '## No tests run.'
    exit 0
}

# Compile first test (nothing to overlap yet).
$compileJob = Start-Compile $testQueue[0]
$currentExe = Wait-Compile $compileJob $testQueue[0]
if ($null -eq $currentExe) { exit 1 }

for ($i = 0; $i -lt $testQueue.Count; $i++) {
    $test = $testQueue[$i]

    # Start compiling next test in background.
    $nextJob = $null
    if ($i + 1 -lt $testQueue.Count) {
        $nextJob = Start-Compile $testQueue[$i + 1]
    }

    # Run current test.
    Write-Host "## Running $($test.File.Name)..."
    $saved = $ErrorActionPreference; $ErrorActionPreference = 'Continue'
    & $currentExe
    $testExit = $LASTEXITCODE
    $ErrorActionPreference = $saved
    $ran++

    if ($testExit -ne 0) {
        Write-Host ''
        Write-Host "## $($test.File.Name) failed."
        if ($null -ne $nextJob) {
            Stop-Job $nextJob -ErrorAction SilentlyContinue
            Remove-Job $nextJob -Force -ErrorAction SilentlyContinue
        }
        $failed = $true
        break
    }
    Remove-Item $currentExe -ErrorAction SilentlyContinue
    Write-Host ''

    # Wait for next compilation.
    if ($null -ne $nextJob) {
        $currentExe = Wait-Compile $nextJob $testQueue[$i + 1]
        if ($null -eq $currentExe) {
            $failed = $true
            break
        }
    }
}

if ($failed) {
    exit 1
} elseif ($ran -gt 0) {
    Write-Host "## All tests passed. ($ran tests)"
} else {
    Write-Host '## No tests run.'
}
