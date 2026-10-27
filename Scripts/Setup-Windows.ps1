param(
 [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8',
 [ValidateSet('Setup','Open','Package')][string]$Mode = 'Setup'
)
$ErrorActionPreference = 'Stop'
$Root = Split-Path $PSScriptRoot -Parent
$Project = Join-Path $Root 'FrontlineCommand.uproject'
$VersionFile = Join-Path $EngineRoot 'Engine\Build\Build.version'
if (!(Test-Path $VersionFile)) { throw "Unreal not found at $EngineRoot. Pass -EngineRoot with your installation folder." }
$V = Get-Content $VersionFile -Raw | ConvertFrom-Json
if ("$($V.MajorVersion).$($V.MinorVersion).$($V.PatchVersion)" -ne '5.8.4') { throw 'This native project targets your reported Unreal Engine 5.8.4. Engine version mismatch.' }
function Run([string]$Exe, [string[]]$Arguments) {
 & $Exe @Arguments
 if ($LASTEXITCODE -ne 0) { throw "Failed ($LASTEXITCODE): $Exe. Check the build output and Saved\Logs." }
}
$Editor = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor.exe'
if ($Mode -eq 'Open') {
 if (!(Test-Path (Join-Path $Root 'Saved\CedarAssets.ready'))) { throw 'Run Setup first to import art and create the map.' }
 Start-Process $Editor -ArgumentList @("`"$Project`"",'/Game/Cedar/Maps/CedarHollow')
 exit 0
}
if ($Mode -eq 'Package') {
 if (!(Test-Path (Join-Path $Root 'Saved\CedarAssets.ready'))) { throw 'Run Setup successfully before packaging.' }
 Run (Join-Path $EngineRoot 'Engine\Build\BatchFiles\RunUAT.bat') @('BuildCookRun',"-project=$Project",'-noP4','-build','-cook','-stage','-pak','-archive','-platform=Win64','-clientconfig=Development',"-archivedirectory=$Root\Builds")
 exit 0
}
if (!(Test-Path (Join-Path $Root 'ArtSource\materials.json'))) { throw 'ArtSource is missing. Download the complete native project, including its art inputs.' }
# Launcher-installed Unreal can build this editor and client. A dedicated server
# is a later, separate source-engine build; it is not a prerequisite for this lab.
Run (Join-Path $EngineRoot 'Engine\Build\BatchFiles\Build.bat') @('FrontlineCommandEditor','Win64','Development',"-Project=$Project",'-WaitMutex')
$Marker = Join-Path $Root 'Saved\CedarAssets.ready'
if (Test-Path $Marker) { Remove-Item $Marker }
$Script = Join-Path $PSScriptRoot 'create-cedar-world.py'
# Render-capable editor: do not use -nullrhi for this material/level build.
Run (Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe') @($Project,"-ExecutePythonScript=$Script",'-unattended','-nosplash','-stdout','-FullStdOutLogOutput')
if (!(Test-Path $Marker)) { throw 'Asset creation did not complete. Read Saved\Logs; the scene is not ready.' }
Write-Host 'Cedar Hollow is prepared. Run this script with -Mode Open, then press Play.'
