param(
    [string]$BuildDir = "build-rescue-sdl2\Debug",
    [string]$DestinationPath = ""
)

$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$buildPath = Resolve-Path (Join-Path $repoRoot $BuildDir)
$gameExe = Join-Path $buildPath "SteambandRedux.exe"
$sourceLib = Join-Path $repoRoot "lib"

if (-not (Test-Path $gameExe)) {
    throw "Missing $gameExe. Build the SDL2 Debug target before packaging."
}

if (-not (Test-Path $sourceLib)) {
    throw "Missing source lib directory at $sourceLib."
}

if ([string]::IsNullOrWhiteSpace($DestinationPath)) {
    $dropRoot = Join-Path $repoRoot "build-rescue-sdl2\rog-ally-drop"
} else {
    $dropRoot = $DestinationPath
}

New-Item -ItemType Directory -Force -Path $dropRoot | Out-Null

$dropExe = Join-Path $dropRoot "SteambandRedux.exe"
$dropLib = Join-Path $dropRoot "lib"
$dropLauncher = Join-Path $dropRoot "launch_topdown_tiles.cmd"

Copy-Item -Force -Path $gameExe -Destination $dropExe

$sdlDebugDll = Join-Path $buildPath "SDL2d.dll"
$sdlReleaseDll = Join-Path $buildPath "SDL2.dll"
if (Test-Path $sdlDebugDll) {
    Copy-Item -Force -Path $sdlDebugDll -Destination (Join-Path $dropRoot "SDL2d.dll")
} elseif (Test-Path $sdlReleaseDll) {
    Copy-Item -Force -Path $sdlReleaseDll -Destination (Join-Path $dropRoot "SDL2.dll")
} else {
    Write-Warning "No SDL2 runtime DLL found beside the executable. The Ally run may fail until the DLL is copied."
}

if (Test-Path $dropLib) {
    Remove-Item -Recurse -Force -Path $dropLib
}
Copy-Item -Recurse -Force -Path $sourceLib -Destination $dropLib

@"
@echo off
setlocal
set "STEAMBAND_LOG_LEVEL=INFO"
set "STEAMBAND_START_TOPDOWN=1"
pushd "%~dp0" >nul
start "" /wait "%~dp0SteambandRedux.exe"
popd >nul
"@ | Set-Content -Encoding ASCII -Path $dropLauncher

$logs = Join-Path $dropLib "logs"
New-Item -ItemType Directory -Force -Path $logs | Out-Null

Write-Host "ROG Ally drop folder ready:"
Write-Host "  $dropRoot"
Write-Host ""
Write-Host "Run on the Ally:"
Write-Host "  SteambandRedux.exe"
Write-Host "  launch_topdown_tiles.cmd  (opens the SDL2 tile window automatically)"
Write-Host ""
Write-Host "Logs will be written under:"
Write-Host "  $(Join-Path $dropLib 'logs')"
