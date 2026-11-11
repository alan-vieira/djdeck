<#
.SYNOPSIS
    Captura a janela de um processo do Windows para PNG (verificacao visual do app).

.DESCRIPTION
    Usa CopyFromScreen (nao PrintWindow) apos restaurar e trazer a janela para
    o primeiro plano. CopyFromScreen e ordens de magnitude mais rapido que
    PrintWindow em janelas grandes (1600x900) e produz exatamente o que o
    usuario ve. Requer que a janela esteja visivel e nao encoberta.

.EXAMPLE
    powershell -NoProfile -ExecutionPolicy Bypass -File tools/capture-window.ps1 `
        -ProcessName djdeck -OutputPath "C:/temp/djdeck.png"
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string] $ProcessName,

    [Parameter(Mandatory = $true)]
    [string] $OutputPath,

    # Rectangulo de recorte em coordenadas de tela (opcional).
    [int] $CropX = 0,
    [int] $CropY = 0,
    [int] $CropW = 0,
    [int] $CropH = 0
)

$ErrorActionPreference = 'Stop'

Add-Type -AssemblyName System.Drawing

Add-Type @"
using System;
using System.Runtime.InteropServices;
public class Win {
  [DllImport("user32.dll")] public static extern bool ShowWindowAsync(IntPtr h, int n);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
}
"@

$procs = Get-Process -Name $ProcessName -ErrorAction SilentlyContinue
if (-not $procs) {
    Write-Error "Processo '$ProcessName' nao encontrado."
    exit 2
}

# Um unico processo: multiplas instanciaspolluem a captura.
$p = $procs | Select-Object -First 1
$hwnd = $p.MainWindowHandle
if ($hwnd -eq [IntPtr]::Zero) {
    Write-Error "Processo '$ProcessName' (PID $($p.Id)) nao tem janela principal."
    exit 2
}

[void][Win]::ShowWindowAsync($hwnd, 9)      # SW_RESTORE
Start-Sleep -Milliseconds 400
[void][Win]::SetForegroundWindow($hwnd)
Start-Sleep -Milliseconds 700

$r = New-Object Win+RECT
if (-not [Win]::GetWindowRect($hwnd, [ref]$r)) {
    Write-Error "GetWindowRect falhou."
    exit 2
}

$w = $r.R - $r.L
$h = $r.B - $r.T
if ($w -le 0 -or $h -le 0) {
    Write-Error "Janela minimizada ou com rect invalido ($w x $h)."
    exit 2
}

Write-Output ("WINDOW     : {0} pid={1} rect={2},{3} {4}x{5}" -f `
    $p.MainWindowTitle, $p.Id, $r.L, $r.T, $w, $h)

$bmp = New-Object System.Drawing.Bitmap($w, $h)
$g = [System.Drawing.Graphics]::FromImage($bmp)
try {
    # PrintWindow com PW_RENDERFULLCONTENT (0x2) captura o CONTEUDO da janela
    # mesmo com outra janela sobreposta. CopyFromScreen seria mais rapido, mas
    # photographa o que esta por cima — o que produz evidencia invalida.
    $hdc = $g.GetHdc()
    try {
        $ok = [Win]::PrintWindow($hwnd, $hdc, 2)
    }
    finally {
        $g.ReleaseHdc($hdc)
    }

    if (-not $ok) {
        Write-Error "PrintWindow falhou para o handle $hwnd."
        exit 2
    }

    Write-Output ("PRINTWINDOW: ok (PW_RENDERFULLCONTENT) {0}x{1}" -f $w, $h)

    if ($CropW -gt 0 -and $CropH -gt 0) {
        $cropRect = New-Object System.Drawing.Rectangle($CropX, $CropY, $CropW, $CropH)
        $cropped = $bmp.Clone($cropRect, $bmp.PixelFormat)
        $bmp.Dispose()
        $bmp = $cropped
        $g.Dispose()
        $g = [System.Drawing.Graphics]::FromImage($bmp)
    }

    $dir = Split-Path -Parent $OutputPath
    if ($dir -and -not (Test-Path -LiteralPath $dir)) {
        New-Item -ItemType Directory -Path $dir -Force | Out-Null
    }

    $bmp.Save($OutputPath, [System.Drawing.Imaging.ImageFormat]::Png)
    Write-Output ("SAVED      : {0} ({1}x{2})" -f $OutputPath, $bmp.Width, $bmp.Height)
}
finally {
    $g.Dispose()
    $bmp.Dispose()
}

exit 0
