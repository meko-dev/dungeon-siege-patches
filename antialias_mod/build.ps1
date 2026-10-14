param([string]$Archive)
$ErrorActionPreference = 'Stop'
$dsBuild = Join-Path $PSScriptRoot 'build'
New-Item -ItemType Directory -Force $dsBuild | Out-Null
if (!$Archive) {
    $Archive = Join-Path $dsBuild 'dgVoodoo2_87_5.zip'
    if (!(Test-Path -LiteralPath $Archive)) {
        Invoke-WebRequest -Uri 'https://dege.freeweb.hu/dgVoodoo2/bin/dgVoodoo2_87_5.zip' -OutFile $Archive
    }
}
if ((Get-FileHash -LiteralPath $Archive).Hash -ne '5FFDE6927F7355CA3FDD5D785B581256A8E6539FA13E395A891ADE6BA1040850') { throw 'Unexpected dgVoodoo archive hash.' }
Add-Type -AssemblyName System.IO.Compression.FileSystem
$dsZip = [IO.Compression.ZipFile]::OpenRead((Resolve-Path -LiteralPath $Archive).Path)
try {
    foreach ($dsName in @('DDraw.dll','D3DImm.dll')) {
        $dsEntry = $dsZip.GetEntry('MS/x86/' + $dsName)
        if (!$dsEntry) { throw "Archive missing $dsName" }
        [IO.Compression.ZipFileExtensions]::ExtractToFile($dsEntry,(Join-Path $dsBuild $dsName),$true)
    }
} finally { $dsZip.Dispose() }
Write-Output 'Prepared dgVoodoo 2.87.5 x86 AA dependencies. See vendor license at https://dege.freeweb.hu/dgVoodoo2/ .'
