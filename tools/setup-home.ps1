# Home setup for Leroy (StackChan CoreS3).
# From the repo root:
#   powershell -ExecutionPolicy Bypass -File tools\setup-home.ps1

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Set-Location $Root

function Require-Python {
    $py = Get-Command python -ErrorAction SilentlyContinue
    if (-not $py) {
        Write-Host 'Python is not on PATH. Install from https://www.python.org/downloads/windows/'
        Write-Host 'Check Add python.exe to PATH, then open a NEW PowerShell.'
        exit 1
    }
    python --version
}

Write-Host '== Python + tools =='
Require-Python
python -m pip install -U pip platformio edge-tts

$example = Join-Path $Root 'src\config.local.h.example'
$local = Join-Path $Root 'src\config.local.h'
if (-not (Test-Path $local)) {
    Copy-Item $example $local
    Write-Host 'Created src\config.local.h - edit WIFI_SSID, WIFI_PASS, OLLAMA_HOST, TTS_HOST'
} else {
    Write-Host 'src\config.local.h already exists'
}

Write-Host ''
Write-Host '== This PC IPv4 addresses (put one in OLLAMA_HOST and TTS_HOST) =='
Get-NetIPAddress -AddressFamily IPv4 |
    Where-Object { $_.IPAddress -notlike '127.*' } |
    Select-Object IPAddress, InterfaceAlias |
    Format-Table -AutoSize

Write-Host 'Next:'
Write-Host '  1. Edit src\config.local.h (Wi-Fi name/password + this PC IPv4 twice)'
Write-Host '  2. Install Ollama from https://ollama.com/download'
Write-Host '     ollama pull llama3.2:3b'
Write-Host '  3. setx OLLAMA_HOST 0.0.0.0'
Write-Host '     Then quit Ollama from the tray and start it again.'
Write-Host '  4. python -m platformio run -e m5stack-cores3 -t upload'
Write-Host '  5. In another window: python tools\tts_server.py'
