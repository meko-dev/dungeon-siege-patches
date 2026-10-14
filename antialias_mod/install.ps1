$ErrorActionPreference = 'Stop'
if (Get-Process DungeonSiege -ErrorAction SilentlyContinue) { throw 'Close Dungeon Siege before installing AA.' }
$dsAaRoot = Split-Path -Parent $PSScriptRoot
$dsAaManifest = Join-Path $PSScriptRoot 'backup\installed.json'
if (Test-Path -LiteralPath $dsAaManifest) { throw 'AA is already installed. Use disable.ps1 before reinstalling.' }
$dsAaSources = @{
    'DDraw.dll' = Join-Path $PSScriptRoot 'build\DDraw.dll'
    'D3DImm.dll' = Join-Path $PSScriptRoot 'build\D3DImm.dll'
    'dgVoodoo.conf' = Join-Path $PSScriptRoot 'dgVoodoo.conf'
}
foreach ($dsAaSource in $dsAaSources.Values) {
    if (!(Test-Path -LiteralPath $dsAaSource)) { throw 'Run antialias_mod/build.ps1 first.' }
}
New-Item -ItemType Directory -Force (Join-Path $PSScriptRoot 'backup') | Out-Null
$dsAaRecords = @()
foreach ($dsAaName in @('DDraw.dll','D3DImm.dll','dgVoodoo.conf')) {
    $dsAaTarget = Join-Path $dsAaRoot $dsAaName
    $dsAaExisted = Test-Path -LiteralPath $dsAaTarget
    if ($dsAaExisted) { Copy-Item -LiteralPath $dsAaTarget -Destination (Join-Path $PSScriptRoot "backup\$dsAaName") -Force }
    $dsAaRecords += [pscustomobject]@{name=$dsAaName; existed=$dsAaExisted; installedHash=(Get-FileHash -LiteralPath $dsAaSources[$dsAaName]).Hash}
}
$dsAaRecords | ConvertTo-Json | Set-Content -LiteralPath $dsAaManifest
foreach ($dsAaName in @('DDraw.dll','D3DImm.dll','dgVoodoo.conf')) {
    Copy-Item -LiteralPath $dsAaSources[$dsAaName] -Destination (Join-Path $dsAaRoot $dsAaName) -Force
}
Write-Output 'Installed dgVoodoo 2.87.5 with 4x MSAA and Direct3D 11 output. Existing version.dll is untouched.'
