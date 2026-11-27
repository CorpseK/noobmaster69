# Builds the FOMOD release zip from build/Release/ImmersiveRecruit.dll and ImmersiveRecruit.ini.
# Usage (after building):  powershell -File package.ps1
# Output: out/ImmersiveRecruit-<version>.zip
#   fomod/                         installer (asks Protected or Essential)
#   Core/SKSE/Plugins/*.dll        always installed
#   Protected/SKSE/Plugins/*.ini   iProtection=1
#   Essential/SKSE/Plugins/*.ini   iProtection=2
$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot
$dll = Join-Path $root 'build\Release\ImmersiveRecruit.dll'
$ini = Join-Path $root 'ImmersiveRecruit.ini'
if (-not (Test-Path $dll)) { throw "Build first: $dll not found" }

$version = ([xml](Get-Content (Join-Path $root 'fomod\info.xml') -Raw)).fomod.Version
$iniText = [IO.File]::ReadAllText($ini)
if ($iniText -notmatch '(?m)^iProtection=1\r?$') { throw 'ImmersiveRecruit.ini must contain iProtection=1' }
$essentialIni = [regex]::Replace($iniText, '(?m)^iProtection=1(\r?)$', 'iProtection=2$1')

$out = Join-Path $root 'out'
New-Item -ItemType Directory -Force $out | Out-Null
$zip = Join-Path $out "ImmersiveRecruit-$version.zip"
if (Test-Path $zip) { Remove-Item $zip -Force }

Add-Type -AssemblyName System.IO.Compression, System.IO.Compression.FileSystem
$z = [IO.Compression.ZipFile]::Open($zip, 'Create')
try {
	function Add-Text($name, $text) {
		$entry = $z.CreateEntry($name)
		$w = New-Object IO.StreamWriter($entry.Open(), (New-Object Text.UTF8Encoding $false))
		$w.Write($text); $w.Dispose()
	}
	[void][IO.Compression.ZipFileExtensions]::CreateEntryFromFile($z, (Join-Path $root 'fomod\info.xml'), 'fomod/info.xml')
	[void][IO.Compression.ZipFileExtensions]::CreateEntryFromFile($z, (Join-Path $root 'fomod\ModuleConfig.xml'), 'fomod/ModuleConfig.xml')
	[void][IO.Compression.ZipFileExtensions]::CreateEntryFromFile($z, $dll, 'Core/SKSE/Plugins/ImmersiveRecruit.dll')
	Add-Text 'Protected/SKSE/Plugins/ImmersiveRecruit.ini' $iniText
	Add-Text 'Essential/SKSE/Plugins/ImmersiveRecruit.ini' $essentialIni
} finally {
	$z.Dispose()
}
Write-Output "Created $zip"
