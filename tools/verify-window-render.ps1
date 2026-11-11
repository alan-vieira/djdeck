<#
.SYNOPSIS
    Prova se a UI de uma janela GUI realmente renderizou, medindo o histograma
    de cores de um screenshot.

.DESCRIPTION
    "Processo vivo" e "MainWindowTitle preenchido" NAO provam que a UI pintou.
    Um app JUCE com filhos 0x0 fica com a janela inteiramente preta e ainda
    responde a eventos. Este script amostra a imagem e imprime as cores
    dominantes. Uma janela renderizada tem varias cores distintas; tela preta e
    quase-monocromatica.

.EXAMPLE
    powershell -NoProfile -ExecutionPolicy Bypass -File tools/verify-window-render.ps1 `
        -ImagePath "C:/temp/djdeck.png" -MinDistinctColors 8
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string] $ImagePath,

    # Abaixo deste numero de cores distintas, considere a janela em branco.
    [int] $MinDistinctColors = 8,

    # Ignora os primeiros N pixels verticais (regiao da title bar).
    [int] $SkipTop = 40,

    # Espacamento entre amostras.
    [int] $Step = 5
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path -LiteralPath $ImagePath)) {
    Write-Error "Imagem nao encontrada: $ImagePath"
    exit 2
}

Add-Type -AssemblyName System.Drawing
$bmp = [System.Drawing.Bitmap]::FromFile($ImagePath)

try {
    $counts = @{}
    $sampled = 0

    for ($y = $SkipTop; $y -lt $bmp.Height; $y += $Step) {
        for ($x = 0; $x -lt $bmp.Width; $x += $Step) {
            $c = $bmp.GetPixel($x, $y)
            $key = "$($c.R),$($c.G),$($c.B)"
            if ($counts.ContainsKey($key)) { $counts[$key] += 1 } else { $counts[$key] = 1 }
            $sampled++
        }
    }

    Write-Output ("IMAGE      : {0} ({1}x{2})" -f (Split-Path $ImagePath -Leaf), $bmp.Width, $bmp.Height)
    Write-Output ("SAMPLED    : {0} pixels" -f $sampled)
    Write-Output "TOP COLORS :"

    foreach ($entry in ($counts.GetEnumerator() | Sort-Object Value -Descending | Select-Object -First 8)) {
        Write-Output ("  {0,-16} -> {1}" -f $entry.Key, $entry.Value)
    }

    $distinct = $counts.Count
    Write-Output ("DISTINCT   : {0} colors" -f $distinct)

    if ($distinct -lt $MinDistinctColors) {
        Write-Output "VERDICT    : TELA PRETA / NAO RENDERIZADA"
        Write-Output "             Os filhos provavelmente nunca receberam bounds (ver resized())."
        exit 1
    }

    Write-Output "VERDICT    : UI RENDERIZADA"
    exit 0
}
finally {
    $bmp.Dispose()
}
