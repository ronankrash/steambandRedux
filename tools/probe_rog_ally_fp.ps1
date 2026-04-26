param(
    [string]$BuildDir = "",
    [int]$StartupTimeoutSeconds = 8,
    [int]$HoldSeconds = 2,
    [switch]$TryNewGame,
    [switch]$TopDown,
    [switch]$AutoTopDown,
    [switch]$BadTileset
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

if ($AutoTopDown) {
    $env:STEAMBAND_START_TOPDOWN = "1"
}
if ($BadTileset) {
    $BadTilesetPath = Join-Path $env:TEMP "steamband_bad_topdown_tileset.bmp"
    [byte[]]$BadBmp = 0x42,0x4D,0x3A,0,0,0,0,0,0,0,0x36,0,0,0,
        0x28,0,0,0,0x01,0,0,0,0x01,0,0,0,0x01,0,0,0,0x18,0,0,0,
        0,0,0,0,0x04,0,0,0,0x13,0x0B,0,0,0x13,0x0B,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0
    [System.IO.File]::WriteAllBytes($BadTilesetPath, $BadBmp)
    $env:STEAMBAND_TOPDOWN_TILESET = $BadTilesetPath
    $env:STEAMBAND_START_TOPDOWN = "1"
    $AutoTopDown = $true
}

$NativeInput = @"
using System;
using System.Runtime.InteropServices;

public static class SteambandProbeInput {
    [DllImport("user32.dll")]
    public static extern bool SetForegroundWindow(IntPtr hWnd);

    [DllImport("user32.dll")]
    public static extern void keybd_event(byte bVk, byte bScan, int dwFlags, int dwExtraInfo);

    [DllImport("user32.dll")]
    public static extern bool PostMessage(IntPtr hWnd, int Msg, IntPtr wParam, IntPtr lParam);

    public const int KEYEVENTF_KEYUP = 0x0002;
    public const int WM_KEYDOWN = 0x0100;
    public const int WM_KEYUP = 0x0101;
}
"@
Add-Type $NativeInput

function Send-KeyDownUp([byte]$VirtualKey) {
    [SteambandProbeInput]::keybd_event($VirtualKey, 0, 0, 0)
    Start-Sleep -Milliseconds 40
    [SteambandProbeInput]::keybd_event($VirtualKey, 0, [SteambandProbeInput]::KEYEVENTF_KEYUP, 0)
}

function Send-Text([string]$Text) {
    foreach ($Char in $Text.ToCharArray()) {
        $Code = [byte][char]::ToUpperInvariant($Char)
        Send-KeyDownUp $Code
        Start-Sleep -Milliseconds 40
    }
}

function Send-CtrlF12([IntPtr]$WindowHandle) {
    [SteambandProbeInput]::keybd_event(0x11, 0, 0, 0) # Ctrl
    Start-Sleep -Milliseconds 40
    [SteambandProbeInput]::PostMessage($WindowHandle,
        [SteambandProbeInput]::WM_KEYDOWN, [IntPtr]0x7B, [IntPtr]0) | Out-Null # F12
    Start-Sleep -Milliseconds 40
    [SteambandProbeInput]::PostMessage($WindowHandle,
        [SteambandProbeInput]::WM_KEYUP, [IntPtr]0x7B, [IntPtr]0) | Out-Null
    [SteambandProbeInput]::keybd_event(0x11, 0, [SteambandProbeInput]::KEYEVENTF_KEYUP, 0)
}

function Send-CtrlF11([IntPtr]$WindowHandle) {
    [SteambandProbeInput]::keybd_event(0x11, 0, 0, 0) # Ctrl
    Start-Sleep -Milliseconds 40
    [SteambandProbeInput]::PostMessage($WindowHandle,
        [SteambandProbeInput]::WM_KEYDOWN, [IntPtr]0x7A, [IntPtr]0) | Out-Null # F11
    Start-Sleep -Milliseconds 40
    [SteambandProbeInput]::PostMessage($WindowHandle,
        [SteambandProbeInput]::WM_KEYUP, [IntPtr]0x7A, [IntPtr]0) | Out-Null
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
    if ($TopDown) {
        # The Win32 title screen can still be reading startup files when the
        # native window first appears. Give Ctrl+F11 the same stable target a
        # handheld tester would have after the prompt is visible.
        Start-Sleep -Milliseconds 1000
    }

    if ($TryNewGame) {
        Send-KeyDownUp 0x4E # N
        Start-Sleep -Milliseconds 500

        # Best-effort default character path:
        # Enter accepts highlighted sex/race/class, accepts the random roll,
        # accepts the typed name, and accepts final character confirmation.
        1..4 | ForEach-Object {
            Send-KeyDownUp 0x0D # Enter
            Start-Sleep -Milliseconds 350
        }
        Send-Text "rogtest"
        Send-KeyDownUp 0x0D # Enter name
        Start-Sleep -Milliseconds 400
        Send-KeyDownUp 0x0D # Continue into world
        Start-Sleep -Milliseconds 900
    }

    if ($TopDown -and !$AutoTopDown) {
        Send-CtrlF11 $Process.MainWindowHandle
    } elseif (!$AutoTopDown) {
        Send-CtrlF12 $Process.MainWindowHandle
    }
    Start-Sleep -Milliseconds 700

    if ($TryNewGame) {
        Send-KeyDownUp 0x57 # W/forward or legacy command
        Start-Sleep -Milliseconds 150
        if (!$TopDown) {
            Send-KeyDownUp 0x44 # D/strafe right
            Start-Sleep -Milliseconds 150
            Send-KeyDownUp 0x25 # Left arrow turn
            Start-Sleep -Milliseconds 150
            Send-KeyDownUp 0x27 # Right arrow turn
            Start-Sleep -Milliseconds 150
        }
    }

    Start-Sleep -Seconds $HoldSeconds

    # Let the SDL window process an exit key if it has focus; CloseMainWindow below is the hard fallback.
    Send-KeyDownUp 0x1B # Esc
    Start-Sleep -Milliseconds 500
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
    $Checks["Title-screen New attempted"] = $true
    $Checks["Default birth keys sent"] = $true
    if ($TopDown -or $AutoTopDown) {
        $Checks["Top-down tile mode activated after New attempt"] = $NewLog.Contains("Top-down SDL tile mode activated")
        if ($BadTileset) {
            $Checks["Bad top-down tilesheet rejected"] = $NewLog.Contains("invalid dimensions") -or
                $NewLog.Contains("top-down tilesheet override failed")
            $Checks["Fallback top-down tilesheet loaded"] = $NewLog.Contains("Loaded SDL2 top-down tilesheet")
        } else {
            $Checks["Top-down tilesheet loaded"] = $NewLog.Contains("Loaded SDL2 top-down tilesheet")
            $Checks["Top-down atlas dimensions verified"] = $NewLog.Contains("SDL2 top-down atlas ready: 216x96 pixels, 9x4 tiles, 36 categories")
        }
    } else {
        $Checks["First-person activated after New attempt"] = $NewLog.Contains("First-person mode activated")
    }
    $Checks["Clean renderer shutdown"] = $NewLog.Contains("Renderer shutdown complete") -or
        $NewLog.Contains("Exited first-person mode") -or
        $NewLog.Contains("Exited SDL renderer mode")
} else {
    if ($TopDown -or $AutoTopDown) {
        $Checks["Top-down tile mode activated"] = $NewLog.Contains("Top-down SDL tile mode activated")
        if ($BadTileset) {
            $Checks["Bad top-down tilesheet rejected"] = $NewLog.Contains("invalid dimensions") -or
                $NewLog.Contains("top-down tilesheet override failed")
            $Checks["Fallback top-down tilesheet loaded"] = $NewLog.Contains("Loaded SDL2 top-down tilesheet")
        } else {
            $Checks["Top-down tilesheet loaded"] = $NewLog.Contains("Loaded SDL2 top-down tilesheet")
            $Checks["Top-down atlas dimensions verified"] = $NewLog.Contains("SDL2 top-down atlas ready: 216x96 pixels, 9x4 tiles, 36 categories")
        }
    } else {
        $Checks["First-person activated"] = $NewLog.Contains("First-person mode activated")
    }
    $Checks["Clean renderer shutdown"] = $NewLog.Contains("Renderer shutdown complete") -or
        $NewLog.Contains("Exited first-person mode") -or
        $NewLog.Contains("Exited SDL renderer mode")
}

if ($TopDown -or $AutoTopDown) {
    Write-Host "SteambandRedux ROG Ally top-down tile automation probe"
} else {
    Write-Host "SteambandRedux ROG Ally first-person automation probe"
}
Write-Host "Process id: $($Process.Id)"
Write-Host "Build dir: $BuildDir"
Write-Host "Log: $GameLog"
foreach ($Check in $Checks.GetEnumerator()) {
    $Status = if ($Check.Value) { "PASS" } else { "FAIL" }
    Write-Host ("{0}: {1}" -f $Status, $Check.Key)
}

if ($TryNewGame) {
    Write-Host "NOTE: -TryNewGame sends default birth-flow keys, then toggles first-person and sends a small movement/turn sequence."
    Write-Host "NOTE: It still cannot visually assert every birth prompt; manual ROG Ally validation remains required."
}

if ($Checks.Values -contains $false) {
    exit 1
}
