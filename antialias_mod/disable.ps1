$ErrorActionPreference = 'Stop'
if (Get-Process DungeonSiege -ErrorAction SilentlyContinue) { throw 'Close Dungeon Siege before disabling AA.' }
$dsAaRoot = [IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot))
$dsAaManifest = Join-Path $PSScriptRoot 'backup\installed.json'
if (Test-Path -LiteralPath $dsAaManifest) {
    $dsAaRecords = Get-Content -LiteralPath $dsAaManifest -Raw | ConvertFrom-Json
} else {
    # Older installations may have had their local backup history cleaned.
    $dsAaHashes = @{
        'DDraw.dll' = '612A24408A090A3C6F3886557FA18034EE742E94AD0A40EBDF854D2816176C2E'
        'D3DImm.dll' = '93C534F2D17419EA78F15551F7E0AAC78B3C503733A840914FA063708A5AFE8E'
    }
    foreach ($dsName in $dsAaHashes.Keys) {
        $dsFile = Join-Path $dsAaRoot $dsName
        if ((Test-Path -LiteralPath $dsFile) -and (Get-FileHash -LiteralPath $dsFile).Hash -ne $dsAaHashes[$dsName]) { throw "Unrecognized wrapper: $dsName" }
    }
    $dsAaRecords = @('DDraw.dll','D3DImm.dll','dgVoodoo.conf') | ForEach-Object { [pscustomobject]@{name=$_;existed=$false} }
}
$dsAaDisabled = Join-Path $PSScriptRoot ('disabled\' + (Get-Date -Format 'yyyyMMdd-HHmmss'))
foreach ($dsAaRecord in $dsAaRecords) {
    if ($dsAaRecord.name -notin @('DDraw.dll','D3DImm.dll','dgVoodoo.conf')) { throw 'Unexpected manifest filename.' }
    $dsAaTarget = [IO.Path]::GetFullPath((Join-Path $dsAaRoot $dsAaRecord.name))
    if ((Split-Path -Parent $dsAaTarget) -ne $dsAaRoot) { throw 'Rollback target is outside the game directory.' }
}
New-Item -ItemType Directory -Force -Path $dsAaDisabled | Out-Null
foreach ($dsAaRecord in $dsAaRecords) {
    $dsAaTarget = Join-Path $dsAaRoot $dsAaRecord.name
    if (Test-Path -LiteralPath $dsAaTarget) { Move-Item -LiteralPath $dsAaTarget -Destination (Join-Path $dsAaDisabled $dsAaRecord.name) }
    if ($dsAaRecord.existed) { Copy-Item -LiteralPath (Join-Path $PSScriptRoot ('backup\' + $dsAaRecord.name)) -Destination $dsAaTarget }
}
if (Test-Path -LiteralPath $dsAaManifest) { Move-Item -LiteralPath $dsAaManifest -Destination (Join-Path $dsAaDisabled 'installed.json') }
Write-Output 'Disabled AA and restored previous wrapper files, if any. Damage numbers and resolution patch remain installed.'
