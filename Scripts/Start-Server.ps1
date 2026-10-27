param([Parameter(Mandatory=$true)][string]$ServerExe, [int]$Port=7777)
$ErrorActionPreference='Stop'
if (!(Test-Path $ServerExe)) { throw 'Pass the path to the packaged FrontlineCommandServer.exe.' }
if ($Port -lt 1024 -or $Port -gt 65535) { throw 'Port must be 1024–65535.' }
& $ServerExe /Engine/Maps/Entry "-port=$Port" -log -unattended
exit $LASTEXITCODE
