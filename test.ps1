param(
    [switch]$g,
    [Parameter(ValueFromRemainingArguments)]
    [string[]]$Remaining
)

$ErrorActionPreference = 'Stop'

$filter = ''
if ($Remaining.Count -gt 0) { $filter = $Remaining[0] }

$projDir = $PSScriptRoot
if (-not $projDir) { $projDir = Get-Location }

# --- Bench command ---
if ($filter -eq 'bench') {
    $benchSrc = "$projDir\bench\bench_coremark.c"
    $benchExe = "$projDir\bench\bench_coremark.exe"
    $benchCflags = @('-W', '-Wall', '-Wextra', '-DWAH_ASSERT=assert', '-O3')
    Write-Host '## Compiling bench_coremark...'
    & clang @benchCflags $benchSrc -o $benchExe
    if ($LASTEXITCODE -ne 0) {
        Write-Host '## Bench compilation failed.'
        exit 1
    }
    Write-Host '## Running CoreMark benchmark...'
    & $benchExe
    $benchExit = $LASTEXITCODE
    Remove-Item $benchExe -ErrorAction SilentlyContinue
    exit $benchExit
}

$cflags = @('-W', '-Wall', '-Wextra')
if ($g) {
    $cflags += '-DWAH_DEBUG', '-g'
} else {
    $cflags += '-DWAH_ASSERT=assert'
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
$wahObj = "$projDir\tests\wah.o"
$commonObj = "$projDir\tests\common.o"

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

# Need wah.o only if there are api-only tests.
$wahObjOk = $true
$wahImplSrc = "$projDir\tests\_wah_impl.c"
if ($apiTests.Count -gt 0) {
    $implContent = "#define WAH_IMPLEMENTATION`n#include `"../wah.h`"`n"
    $needsWrite = $true
    if (Test-Path $wahImplSrc) {
        $existing = [System.IO.File]::ReadAllText($wahImplSrc)
        if ($existing -eq $implContent) { $needsWrite = $false }
    }
    if ($needsWrite) { [System.IO.File]::WriteAllText($wahImplSrc, $implContent) }

    $wahObjOk = Test-ObjUpToDate $wahObj @($wahH, $wahImplSrc)
}

$jobs = @()
if (-not $wahObjOk) {
    Write-Host '## Compiling wah.o...'
    $jobs += Start-Job -ScriptBlock {
        param($cflags, $src, $out, $dir)
        Set-Location $dir
        & clang @cflags -c $src -o $out 2>&1
        if ($LASTEXITCODE -ne 0) { throw 'wah.o compilation failed' }
    } -ArgumentList $cflags, $wahImplSrc, $wahObj, $projDir
}
if (-not $commonObjOk) {
    Write-Host '## Compiling common.o...'
    $jobs += Start-Job -ScriptBlock {
        param($cflags, $src, $out, $dir)
        Set-Location $dir
        & clang @cflags -c $src -o $out 2>&1
        if ($LASTEXITCODE -ne 0) { throw 'common.o compilation failed' }
    } -ArgumentList $cflags, $commonSrc, $commonObj, $projDir
}

$objFailed = $false
foreach ($j in $jobs) {
    $null = Receive-Job $j -Wait -ErrorAction SilentlyContinue
    if ($j.State -eq 'Failed') {
        Write-Host "## Precompiled object build failed."
        $objFailed = $true
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

    # standalone: test.o has WAH_IMPLEMENTATION, link with common.o
    # api-only: test.o has no WAH_IMPLEMENTATION, link with wah.o + common.o
    $linkObjStr = if ($test.Standalone) { $commonObj } else { "$wahObj|$commonObj" }

    return Start-Job -ScriptBlock {
        param($cflags, $src, $exe, $linkObjStr, $dir)
        Set-Location $dir
        $linkObjs = $linkObjStr -split '\|'
        $obj = $exe -replace '\.exe$', '.o'
        & clang @cflags -c $src -o $obj 2>&1
        if ($LASTEXITCODE -ne 0) { $LASTEXITCODE; return }
        & clang $obj @linkObjs -o $exe 2>&1
        $lec = $LASTEXITCODE
        Remove-Item $obj -ErrorAction SilentlyContinue
        $lec
    } -ArgumentList $script:cflags, $f.FullName, $exe, $linkObjStr, $script:projDir
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
