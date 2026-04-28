param(
    [string]$Preset = "x64-release-qt",
    [string]$Version = "v0.9.0",
    [string]$VsDevCmd = "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat"
)

$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$buildDir = Join-Path $repoRoot "out\build\$Preset\GeoTaskShield"
$packageRoot = Join-Path $repoRoot "out\package"
$stageDir = Join-Path $packageRoot "GeoTaskShield-$Version-windows-x64"
$zipPath = Join-Path $packageRoot "GeoTaskShield-$Version-windows-x64.zip"

Push-Location $repoRoot
try {
    if (Test-Path -LiteralPath $VsDevCmd) {
        $command = "call `"$VsDevCmd`" -arch=x64 && cmake --preset $Preset && cmake --build out\build\$Preset"
        & cmd /c $command
    } else {
        cmake --preset $Preset
        cmake --build "out\build\$Preset"
    }
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed with exit code $LASTEXITCODE."
    }

    if (!(Test-Path -LiteralPath $buildDir)) {
        throw "Build output directory not found: $buildDir"
    }

    if (Test-Path -LiteralPath $stageDir) {
        Remove-Item -LiteralPath $stageDir -Recurse -Force
    }
    New-Item -ItemType Directory -Path $stageDir | Out-Null

    $appFiles = @(
        "GeoTaskShield.exe",
        "GeoTaskShieldAgentDemo.exe",
        "GeoTaskShieldBatchDemo.exe",
        "GeoTaskShieldGui.exe"
    )

    foreach ($file in $appFiles) {
        Copy-Item -LiteralPath (Join-Path $buildDir $file) -Destination $stageDir
    }

    Get-ChildItem -LiteralPath $buildDir -Filter "*.dll" -File |
        Copy-Item -Destination $stageDir

    $pluginDirs = @(
        "generic",
        "iconengines",
        "imageformats",
        "networkinformation",
        "platforms",
        "styles",
        "tls"
    )

    foreach ($dir in $pluginDirs) {
        $source = Join-Path $buildDir $dir
        if (Test-Path -LiteralPath $source) {
            Copy-Item -LiteralPath $source -Destination $stageDir -Recurse
        }
    }

    $docFiles = @(
        "README.md",
        "HANDOFF.md",
        "CHANGELOG.md",
        "phase2_results.csv",
        "phase5_batch_results.csv",
        "phase5_batch_report.md"
    )

    foreach ($file in $docFiles) {
        $source = Join-Path $repoRoot $file
        if (Test-Path -LiteralPath $source) {
            Copy-Item -LiteralPath $source -Destination $stageDir
        }
    }

    $docDirs = @(
        "docs\demo"
    )

    foreach ($dir in $docDirs) {
        $source = Join-Path $repoRoot $dir
        if (Test-Path -LiteralPath $source) {
            $destination = Join-Path $stageDir $dir
            New-Item -ItemType Directory -Path (Split-Path -Parent $destination) -Force | Out-Null
            Copy-Item -LiteralPath $source -Destination $destination -Recurse
        }
    }

    if (Test-Path -LiteralPath $zipPath) {
        Remove-Item -LiteralPath $zipPath -Force
    }
    Compress-Archive -Path (Join-Path $stageDir "*") -DestinationPath $zipPath -Force

    Write-Host "Package created: $zipPath"
} finally {
    Pop-Location
}
