# Run from PowerShell with GCC available on PATH (for example MinGW-w64).
$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    New-Item -ItemType Directory -Force -Path build | Out-Null
    $sources = @(Get-ChildItem src -Filter *.c | ForEach-Object { $_.FullName })
    & gcc -std=c11 -Wall -Wextra -Wpedantic -g -Iinclude @sources -o build/pizza-ordering-system.exe
    if ($LASTEXITCODE -ne 0) { throw 'C compilation failed.' }
} finally {
    Pop-Location
}
