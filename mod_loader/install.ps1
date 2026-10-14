$ErrorActionPreference = 'Stop'
if (Get-Process DungeonSiege -ErrorAction SilentlyContinue) { throw 'Close Dungeon Siege before installing.' }
$dsRoot = Split-Path -Parent $PSScriptRoot
if ((Get-FileHash -LiteralPath (Join-Path $dsRoot 'DungeonSiege.exe')).Hash -ne 'C408EF77B39484D8AD82BA17859CF1E60B24D3BAF6D429283A52B886D67F33AB') { throw 'Unsupported game executable.' }
$dsSource = Join-Path $PSScriptRoot 'build\version.dll'
$dsTarget = Join-Path $dsRoot 'version.dll'
$dsBackup = Join-Path $PSScriptRoot 'backup\version.before-split.dll'
New-Item -ItemType Directory -Force (Split-Path -Parent $dsBackup) | Out-Null
if (!(Test-Path -LiteralPath $dsBackup) -and (Test-Path -LiteralPath $dsTarget)) { Copy-Item -LiteralPath $dsTarget -Destination $dsBackup }
Copy-Item -LiteralPath $dsSource -Destination $dsTarget -Force
Write-Output 'Installed shared version.dll loader. Install each plugin with its own install.ps1.'
