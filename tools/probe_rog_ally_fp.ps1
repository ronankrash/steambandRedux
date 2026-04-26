param(
    [string]$BuildDir = "",
    [int]$StartupTimeoutSeconds = 8,
    [int]$HoldSeconds = 2,
    [switch]$TryNewGame
)

$ErrorActionPreference = "Stop"

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
if ([string]::IsNullOrWhiteSpace($BuildDir)) {
    $BuildDir = Join-Path $RepoRoot "build-rescue-sdl2\Debug"
}
$BuildDir = (Resolve-Path $BuildDir).Path
$GameExe = Join-Path $BuildDir "SteambandRedux.exe"
$GameLog = Join-Path $BuildDir "lib\logs\steamband.log"

if (!(Test-Path $GameExe)) {
    throw "Missing executable: $GameExe"
}

$NativeInput = @"
using System;
using System.Runtime.InteropServices;

public static class SteambandProbeInput {
    [DllImport("user32.dll")]
    public static extern bool SetForegroundWindow(IntPtr hWnd);

    [DllImport("user32.dll")]
    public static extern void keybd_event(byte bVk, byte bScan, int dwFlags, int dwExtraInfo);

    public const int KEYEVENTF_KEYUP = 0x0002;
}
"@
Add-Type $NativeInput

function Send-KeyDownUp([byte]$VirtualKey) {
    [SteambandProbeInput]::keybd_event($VirtualKey, 0, 0, 0)
    Start-Sleep -Milliseconds 40
    [SteambandProbeInput]::keybd_event($VirtualKey, 0, [SteambandProbeInput]::KEYEVENTF_KEYUP, 0)
}

function Send-CtrlF12 {
    [SteambandProbeInput]::keybd_event(0x11, 0, 0, 0) # Ctrl
    Start-Sleep -Milliseconds 40
    [SteambandProbeInput]::keybd_event(0x7B, 0, 0, 0) # F12
    Start-Sleep -Milliseconds 40
    [SteambandProbeInput]::keybd_event(0x7B, 0, [SteambandProbeInput]::KEYEVENTF_KEYUP, 0)
    [SteambandProbeInput]::keybd_event(0x11, 0, [SteambandProbeInput]::KEYEVENTF_KEYUP, 0)
}

$PreviousLogLength = 0
if (Test-Path $GameLog) {
    $PreviousLogLength = (Get-Item $GameLog).Length
}

$Process = Start-Process -FilePath $GameExe -WorkingDirectory $BuildDir -PassThru
try {
    $Deadline = (Get-Date).AddSeconds($StartupTimeoutSeconds)
    while ($Process.MainWindowHandle -eq 0 -and (Get-Date) -lt $Deadline) {
        Start-Sleep -Milliseconds 250
        $Process.Refresh()
    }

    if ($Process.MainWindowHandle -eq 0) {
        throw "Game process started but no main window appeared before timeout."
    }

    [SteambandProbeInput]::SetForegroundWindow($Process.MainWindowHandle) | Out-Null
    Start-Sleep -Milliseconds 300

    if ($TryNewGame) {
        Send-KeyDownUp 0x4E # N
        Start-Sleep -Milliseconds 700
        # Best effort only: these legacy birth prompts are visual and still need manual confirmation.
        Send-KeyDownUp 0x1B # Esc, commonly accepts defaults/backtracks in birth prompts
        Start-Sleep -Milliseconds 200
    } else {
        Send-CtrlF12
        Start-Sleep -Seconds $HoldSeconds

        # Let the SDL window process an exit key if it has focus; CloseMainWindow below is the hard fallback.
        Send-KeyDownUp 0x1B # Esc
        Start-Sleep -Milliseconds 500
    }
}
finally {
    if (!$Process.HasExited) {
        $Process.CloseMainWindow() | Out-Null
        Start-Sleep -Milliseconds 700
    }
    if (!$Process.HasExited) {
        Stop-Process -Id $Process.Id -Force
    }
}

$NewLog = ""
if (Test-Path $GameLog) {
    $Stream = [System.IO.File]::Open($GameLog,
        [System.IO.FileMode]::Open,
        [System.IO.FileAccess]::Read,
        [System.IO.FileShare]::ReadWrite)
    try {
        $Bytes = New-Object byte[] $Stream.Length
        [void]$Stream.Read($Bytes, 0, $Bytes.Length)
    }
    finally {
        $Stream.Close()
    }

    if ($Bytes.Length -gt $PreviousLogLength) {
        $NewLog = [System.Text.Encoding]::UTF8.GetString($Bytes, $PreviousLogLength, $Bytes.Length - $PreviousLogLength)
    }
}

$Checks = [ordered]@{
    "Main window appeared" = $true
    "Renderer initialized" = $NewLog.Contains("Renderer initialized hidden")
    "DDA startup logged" = $NewLog.Contains("Renderer DDA test")
}

if ($TryNewGame) {
    $Checks["Title-screen New injected"] = $true
    $Checks["Clean renderer shutdown"] = $NewLog.Contains("Renderer shutdown complete") -or $NewLog.Contains("Exited first-person mode")
} else {
    $Checks["First-person activated"] = $NewLog.Contains("First-person mode activated")
    $Checks["Clean renderer shutdown"] = $NewLog.Contains("Renderer shutdown complete") -or $NewLog.Contains("Exited first-person mode")
}

Write-Host "SteambandRedux ROG Ally first-person automation probe"
Write-Host "Process id: $($Process.Id)"
Write-Host "Build dir: $BuildDir"
Write-Host "Log: $GameLog"
foreach ($Check in $Checks.GetEnumerator()) {
    $Status = if ($Check.Value) { "PASS" } else { "FAIL" }
    Write-Host ("{0}: {1}" -f $Status, $Check.Key)
}

if ($TryNewGame) {
    Write-Host "NOTE: -TryNewGame injects the title-screen New command only and does not run the first-person activation check."
    Write-Host "NOTE: Character creation remains manual unless future probes can read UI state."
}

if ($Checks.Values -contains $false) {
    exit 1
}
