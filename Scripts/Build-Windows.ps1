# Compatibility entry point; native benchmark is now UE 5.8.4.
param([string]$EngineRoot='C:\Program Files\Epic Games\UE_5.8',[ValidateSet('Build','Assets','Package')][string]$Mode='Build')
$ErrorActionPreference='Stop'
$NewMode=if($Mode -eq 'Package'){'Package'}else{'Setup'}
& (Join-Path $PSScriptRoot 'Setup-Windows.ps1') -EngineRoot $EngineRoot -Mode $NewMode
exit $LASTEXITCODE
