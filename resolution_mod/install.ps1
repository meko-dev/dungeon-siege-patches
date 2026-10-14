$ErrorActionPreference = 'Stop'
$dsGameRoot = Split-Path -Parent $PSScriptRoot
if (Get-Process DungeonSiege -ErrorAction SilentlyContinue) { throw 'Close Dungeon Siege before installing.' }
if ((Get-FileHash -LiteralPath (Join-Path $dsGameRoot 'DungeonSiege.exe')).Hash -ne 'C408EF77B39484D8AD82BA17859CF1E60B24D3BAF6D429283A52B886D67F33AB') { throw 'Unsupported game executable.' }
$dsExpectedLoader = Join-Path $dsGameRoot 'mod_loader\build\version.dll'
if (!(Test-Path -LiteralPath $dsExpectedLoader) -or (Get-FileHash -LiteralPath (Join-Path $dsGameRoot 'version.dll')).Hash -ne (Get-FileHash -LiteralPath $dsExpectedLoader).Hash) { throw 'Install mod_loader\install.ps1 first.' }
$dsTarget = Join-Path $dsGameRoot 'ds_resolution.dll'
if (Test-Path -LiteralPath $dsTarget) {
    New-Item -ItemType Directory -Force (Join-Path $PSScriptRoot 'backup') | Out-Null
    Copy-Item -LiteralPath $dsTarget -Destination (Join-Path $PSScriptRoot ('backup\ds_resolution.' + (Get-Date -Format 'yyyyMMdd-HHmmss-fff') + '.dll'))
}
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'build\ds_resolution.dll') -Destination $dsTarget -Force
Write-Output 'Installed ds_resolution.dll. Shared loader and other mods preserved.'
