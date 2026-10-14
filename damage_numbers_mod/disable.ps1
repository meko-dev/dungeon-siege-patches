$ErrorActionPreference = 'Stop'
if (Get-Process DungeonSiege -ErrorAction SilentlyContinue) { throw 'Close Dungeon Siege before disabling.' }
$dsTarget = Join-Path (Split-Path -Parent $PSScriptRoot) 'ds_damage_numbers.dll'
if (Test-Path -LiteralPath $dsTarget) {
    $dsDisabled = Join-Path $PSScriptRoot 'disabled'
    New-Item -ItemType Directory -Force $dsDisabled | Out-Null
    Move-Item -LiteralPath $dsTarget -Destination (Join-Path $dsDisabled ('ds_damage_numbers.' + (Get-Date -Format 'yyyyMMdd-HHmmss-fff') + '.dll'))
}
Write-Output 'Disabled ds_damage_numbers; the other mods remain installed.'
