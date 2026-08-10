[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'

# Ricava i percorsi dalla posizione di questo script.
$repositoryRoot = Split-Path -Parent $PSScriptRoot
$projectPath = Join-Path $repositoryRoot 'UE57_Accessible_Lab.uproject'
$logDirectory = Join-Path $repositoryRoot 'Saved\Logs'
$logPath = Join-Path $logDirectory 'AccessibleBuild.log'

$exitCode = 1
$failureMessage = $null

try {
    New-Item -ItemType Directory -Path $logDirectory -Force | Out-Null

    # UE_ROOT ha precedenza sull'installazione standard di Unreal Engine 5.7.
    $engineCandidates = @()
    if (-not [string]::IsNullOrWhiteSpace($env:UE_ROOT)) {
        $engineCandidates += $env:UE_ROOT
    }
    $engineCandidates += 'C:\Program Files\Epic Games\UE_5.7'

    $buildScript = $null
    foreach ($engineRoot in $engineCandidates) {
        $candidate = Join-Path $engineRoot 'Engine\Build\BatchFiles\Build.bat'
        if (Test-Path -LiteralPath $candidate -PathType Leaf) {
            $buildScript = $candidate
            break
        }
    }

    if ($null -eq $buildScript) {
        throw "Unreal Engine 5.7 non trovato. Impostare UE_ROOT o installarlo in C:\Program Files\Epic Games\UE_5.7."
    }
    if (-not (Test-Path -LiteralPath $projectPath -PathType Leaf)) {
        throw "File di progetto non trovato: $projectPath"
    }

    # Unisce i flussi per conservare nel log tutto l'output della compilazione.
    & $buildScript 'UE57_Accessible_LabEditor' 'Win64' 'Development' $projectPath 2>&1 |
        Tee-Object -FilePath $logPath
    $exitCode = $LASTEXITCODE
}
catch {
    $failureMessage = $_.Exception.Message
    $failureMessage | Out-File -LiteralPath $logPath -Encoding UTF8
    $exitCode = 1
}

if ($exitCode -eq 0) {
    Write-Host ''
    Write-Host 'BUILD RIUSCITA'
}
else {
    # Estrae un riepilogo breve, utile anche con tecnologie assistive.
    $errorLines = @(Select-String -LiteralPath $logPath -Pattern 'fatal error|error|exception' |
        ForEach-Object { $_.Line } |
        Select-Object -Unique)

    if ($errorLines.Count -eq 0 -and -not [string]::IsNullOrWhiteSpace($failureMessage)) {
        $errorLines = @($failureMessage)
    }

    $summary = if ($errorLines.Count -gt 0) {
        $errorLines -join [Environment]::NewLine
    }
    else {
        'Nessuna riga di errore specifica trovata. Consultare il log completo.'
    }

    Write-Host ''
    Write-Host 'BUILD FALLITA'
    Write-Host 'Riepilogo errori:'
    Write-Host $summary

    # Copia il riepilogo solo quando il comando è disponibile.
    if (Get-Command Set-Clipboard -ErrorAction SilentlyContinue) {
        $clipboardError = $null
        $summary | Set-Clipboard -ErrorAction SilentlyContinue -ErrorVariable clipboardError
        if ($null -eq $clipboardError) {
            Write-Host 'Riepilogo copiato negli appunti.'
        }
    }
}

exit $exitCode
