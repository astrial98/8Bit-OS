# Character-Based Native PowerShell Serial Terminal for 8-Bit Hex OS
$portName = "COM7"
$baudRate = 9600

try {
    $port = New-Object System.IO.Ports.SerialPort $portName, $baudRate, None, 8, one
    $port.DtrEnable = $true; $port.RtsEnable = $true
    $port.ReadTimeout = 50; $port.WriteTimeout = 500
    $port.Open()
    [System.Threading.Thread]::Sleep(1500) 
} catch {
    Write-Host "ERROR: Windows blocked access to $portName!" -ForegroundColor Red
    Exit
}

Clear-Host
Write-Host "==================================================" -ForegroundColor Cyan
Write-Host "   8-BIT NATIVE POWERSHELL TERMINAL ACTIVATED     " -ForegroundColor Cyan
Write-Host "==================================================`n" -ForegroundColor Cyan

$global:interactiveMode = $false
$commandBuffer = ""
$lineBuffer = "" 

try {
    while ($port.IsOpen) {
        while ($port.BytesToRead -gt 0) {
            try {
                $charInt = $port.ReadChar()
                $charStr = [char]$charInt
                if ($charStr -eq "`n") {
                    Write-Host ""; $lineBuffer = "" 
                } elseif ($charStr -eq "`r") {
                    # Skip
                } else {
                    Write-Host -NoNewline $charStr -ForegroundColor White
                    $lineBuffer += $charStr
                }
                if ($lineBuffer -match "Type:") { $global:interactiveMode = $true; $lineBuffer = "" } 
                elseif ($lineBuffer -match "> ") { $global:interactiveMode = $false; $lineBuffer = ""; $commandBuffer = "" }
            } catch [TimeoutException] {}
        }

        if ([System.Console]::KeyAvailable) {
            $keyInfo = [System.Console]::ReadKey($true)
            if ($global:interactiveMode) {
                $charToSend = $keyInfo.KeyChar
                Write-Host $charToSend -NoNewline -ForegroundColor Yellow
                $port.Write($charToSend)
                $global:interactiveMode = $false 
            } else {
                if ($keyInfo.Key -eq [System.ConsoleKey]::Enter) {
                    Write-Host "" 
                    if ($port.IsOpen -and $commandBuffer) { $port.WriteLine($commandBuffer) }
                    $commandBuffer = "" 
                } elseif ($keyInfo.Key -eq [System.ConsoleKey]::Backspace) {
                    if ($commandBuffer.Length -gt 0) {
                        $commandBuffer = $commandBuffer.Substring(0, $commandBuffer.Length - 1)
                        Write-Host -NoNewline "`b `b"
                    }
                } else {
                    $char = $keyInfo.KeyChar
                    Write-Host -NoNewline $char -ForegroundColor Gray
                    $commandBuffer += $char
                }
            }
        }
        [System.Threading.Thread]::Sleep(15)
    }
} finally {
    if ($port.IsOpen) { $port.Close() }
}
