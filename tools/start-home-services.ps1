# Run after Ollama is installed. Leaves TTS in the foreground.
#   powershell -ExecutionPolicy Bypass -File tools\start-home-services.ps1

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Set-Location $Root

if (-not $env:OLLAMA_HOST) {
    $env:OLLAMA_HOST = "0.0.0.0"
}

Write-Host "OLLAMA_HOST=$env:OLLAMA_HOST"
if (Get-Command ollama -ErrorAction SilentlyContinue) {
    Write-Host "Pulling llama3.2:3b if needed..."
    ollama pull llama3.2:3b
} else {
    Write-Host "Ollama not on PATH. Install from https://ollama.com/download"
}

Write-Host "Starting Andrew TTS on port 43146. Leave this window open."
python tools\tts_server.py
