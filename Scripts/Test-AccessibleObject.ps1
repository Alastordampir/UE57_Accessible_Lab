[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'

$repositoryRoot = Split-Path -Parent $PSScriptRoot
$projectPath = Join-Path $repositoryRoot 'UE57_Accessible_Lab.uproject'
$buildScript = Join-Path $PSScriptRoot 'Build-Lab.ps1'
$reportDirectory = Join-Path $repositoryRoot 'Saved\TestReports\AccessibleObject'
$reportPath = Join-Path $reportDirectory 'index.json'
$logDirectory = Join-Path $repositoryRoot 'Saved\Logs'
$logPath = Join-Path $logDirectory 'AccessibleTests.log'
$testFilter = 'UE57AccessibleLab.Accessibility.AccessibleObject'
$expectedTests = @(
    "$testFilter.Defaults"
    "$testFilter.Announcement"
    "$testFilter.Availability"
)

$failureReason = $null

try {
    if (-not (Test-Path -LiteralPath $buildScript -PathType Leaf)) {
        throw "Script di build non trovato: $buildScript"
    }

    & $buildScript
    if ($LASTEXITCODE -ne 0) {
        throw "Build fallita con exit code $LASTEXITCODE."
    }

    $engineCandidates = @()
    if (-not [string]::IsNullOrWhiteSpace($env:UE_ROOT)) {
        $engineCandidates += $env:UE_ROOT
    }
    $engineCandidates += 'C:\Program Files\Epic Games\UE_5.7'

    $editorCommand = $null
    foreach ($engineRoot in $engineCandidates) {
        $candidate = Join-Path $engineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
        if (Test-Path -LiteralPath $candidate -PathType Leaf) {
            $editorCommand = $candidate
            break
        }
    }

    if ($null -eq $editorCommand) {
        throw 'UnrealEditor-Cmd.exe di Unreal Engine 5.7 non trovato. Impostare UE_ROOT o installarlo in C:\Program Files\Epic Games\UE_5.7.'
    }
    if (-not (Test-Path -LiteralPath $projectPath -PathType Leaf)) {
        throw "File di progetto non trovato: $projectPath"
    }

    if (Test-Path -LiteralPath $reportDirectory) {
        Remove-Item -LiteralPath $reportDirectory -Recurse -Force
    }
    New-Item -ItemType Directory -Path $reportDirectory -Force | Out-Null
    New-Item -ItemType Directory -Path $logDirectory -Force | Out-Null

    $editorArguments = @(
        $projectPath
        '-unattended'
        '-nop4'
        '-nosplash'
        '-nullrhi'
        "-ExecCmds=Automation RunTest $testFilter; Quit"
        "-ReportExportPath=$reportDirectory"
        "-abslog=$logPath"
    )

    & $editorCommand $editorArguments
    $editorExitCode = $LASTEXITCODE

    if (-not (Test-Path -LiteralPath $reportPath -PathType Leaf)) {
        throw "Report JSON mancante: $reportPath. Exit code Unreal: $editorExitCode."
    }

    $report = Get-Content -LiteralPath $reportPath -Raw | ConvertFrom-Json
    $reportedTests = @($report.tests)

    if ($reportedTests.Count -ne $expectedTests.Count) {
        throw "Numero di test eseguiti non valido: attesi $($expectedTests.Count), trovati $($reportedTests.Count)."
    }

    foreach ($expectedTest in $expectedTests) {
        $matchingTests = @($reportedTests | Where-Object { $_.fullTestPath -eq $expectedTest })
        if ($matchingTests.Count -ne 1) {
            throw "Test mancante o duplicato nel report: $expectedTest."
        }
        if ($matchingTests[0].state -ne 'Success') {
            throw "Test non superato: $expectedTest (stato: $($matchingTests[0].state))."
        }
    }

    $failedCount = [int]$report.failed
    $notRunCount = [int]$report.notRun
    $inProcessCount = [int]$report.inProcess
    $successfulCount = [int]$report.succeeded + [int]$report.succeededWithWarnings
    if ($failedCount -ne 0 -or $notRunCount -ne 0 -or $inProcessCount -ne 0 -or $successfulCount -ne $expectedTests.Count) {
        throw "Riepilogo report non valido: superati=$successfulCount, falliti=$failedCount, non eseguiti=$notRunCount, incompleti=$inProcessCount."
    }

    if ($editorExitCode -ne 0) {
        throw "UnrealEditor-Cmd.exe ha restituito exit code $editorExitCode nonostante il report sia presente."
    }
}
catch {
    $failureReason = $_.Exception.Message
}

if ($null -eq $failureReason) {
    Write-Host ''
    Write-Host 'TEST ACCESSIBILITA RIUSCITI'
    exit 0
}

Write-Host ''
Write-Host 'TEST ACCESSIBILITA FALLITI'
Write-Host "Errore: $failureReason"
Write-Host "Log: $logPath"
Write-Host "Report: $reportPath"
exit 1
