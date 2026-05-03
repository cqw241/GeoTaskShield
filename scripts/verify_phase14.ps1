param(
    [string]$VsDevCmd = "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat",
    [switch]$SkipQt
)

$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path

function Invoke-VerificationCommand {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Command
    )

    if (Test-Path -LiteralPath $VsDevCmd) {
        & cmd /c "call `"$VsDevCmd`" -arch=x64 && $Command"
    } else {
        & cmd /c $Command
    }

    if ($LASTEXITCODE -ne 0) {
        throw "Verification command failed with exit code $LASTEXITCODE`: $Command"
    }
}

Push-Location $repoRoot
try {
    Invoke-VerificationCommand "cmake --preset x64-debug && cmake --build out\build\x64-debug && ctest --test-dir out\build\x64-debug --output-on-failure"

    if (-not $SkipQt) {
        Invoke-VerificationCommand "cmake --preset x64-debug-qt && cmake --build out\build\x64-debug-qt && ctest --test-dir out\build\x64-debug-qt --output-on-failure"
    }
} finally {
    Pop-Location
}
