$port= new-Object System.IO.Ports.SerialPort COM3,9600,None,8,one
$port.open()
$port.WriteLine("Time:{0}" -f [Math]::Floor([decimal](Get-Date(Get-Date).ToUniversalTime()-uformat "%s")))
$port.Close()