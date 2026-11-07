# Member A: Windows GCC build, isolated demo and regression test entry points.
param(
    [ValidateSet('build', 'demo', 'test')]
    [string]$Mode = 'build'
)
$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    if (-not (Get-Command gcc -ErrorAction SilentlyContinue)) {
        throw 'GCC was not found. Install MinGW-w64 and add its bin directory to PATH.'
    }
    New-Item -ItemType Directory -Force -Path build | Out-Null
    $flags = @('-std=c11', '-Wall', '-Wextra', '-Wpedantic', '-g', '-Iinclude')
    $sources = @(Get-ChildItem src -Filter *.c | ForEach-Object { $_.FullName })
    & gcc @flags @sources -o build/pizza-ordering-system.exe
    if ($LASTEXITCODE -ne 0) { throw 'Application compilation failed.' }
    if ($Mode -eq 'demo' -or $Mode -eq 'test') {
        & gcc @flags -DPIZZA_DEMO src/main.c src/ui.c src/app.c src/input.c tests/demo_modules.c -o build/pizza-demo.exe
        if ($LASTEXITCODE -ne 0) { throw 'Demo compilation failed.' }
    }
    if ($Mode -eq 'test') {
        & gcc @flags tests/test_core.c src/app.c src/input.c tests/demo_modules.c -o build/test-core.exe
        if ($LASTEXITCODE -ne 0) { throw 'Core test compilation failed.' }
        & ./build/test-core.exe
        if ($LASTEXITCODE -ne 0) { throw 'Core tests failed.' }
        $pythonCommand = Get-Command python -ErrorAction SilentlyContinue
        if (-not $pythonCommand) { $pythonCommand = Get-Command python3 -ErrorAction SilentlyContinue }
        if (-not $pythonCommand) { throw 'Install Python 3 to run the end-to-end tests.' }
        & $pythonCommand.Source tests/test_app.py --build-dir build
        if ($LASTEXITCODE -ne 0) { throw 'Application tests failed.' }
    }
    Write-Host "Build completed ($Mode). Executables are in build/."
} finally {
    Pop-Location
}
