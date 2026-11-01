#Requires -Version 7.2
# GPL-2.0-or-later. Hardware validation for the pinned ATmega328PB Mini image.
# Requires PowerShell 7.2+. The default mode builds probes and reads backups only.
[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$Port,
    [string]$AvrdudePath = 'avrdude',
    [string]$AvrdudeConfig = '',
    [string]$OutputDirectory = '',
    [switch]$UseWslToolchain,
    [string]$RestoreFromDirectory = '',
    [switch]$ColdStart,
    [switch]$Run
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$sourceDir = $PSScriptRoot
$repoRoot = [IO.Path]::GetFullPath((Join-Path $sourceDir '../../..'))
$Baud = 115200
$restoreOnly = -not [string]::IsNullOrWhiteSpace($RestoreFromDirectory)
if ($restoreOnly) { $taskDir = [IO.Path]::GetFullPath($RestoreFromDirectory) }
else {
    if ([string]::IsNullOrWhiteSpace($OutputDirectory)) {
        $OutputDirectory = Join-Path $repoRoot ('.worktree/nano-usb-test-' + [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfff'))
    }
    $taskDir = [IO.Path]::GetFullPath($OutputDirectory)
    $rootPrefix = $repoRoot.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar
    $privatePrefix = [IO.Path]::GetFullPath((Join-Path $repoRoot '.worktree')).TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar
    if ($taskDir.StartsWith($rootPrefix,[StringComparison]::OrdinalIgnoreCase) -and
        -not $taskDir.StartsWith($privatePrefix,[StringComparison]::OrdinalIgnoreCase)) {
        throw 'Device backups inside this checkout must be under the ignored .worktree directory.'
    }
    if (Test-Path -LiteralPath $taskDir) { throw 'OutputDirectory must not exist; existing backups are never overwritten.' }
    New-Item -ItemType Directory -Path $taskDir -Force | Out-Null
}
$avrExe = (Get-Command $AvrdudePath -CommandType Application -ErrorAction Stop).Source
if ([string]::IsNullOrWhiteSpace($AvrdudeConfig)) {
    $candidate = [IO.Path]::GetFullPath((Join-Path (Split-Path $avrExe -Parent) '../etc/avrdude.conf'))
    if (Test-Path -LiteralPath $candidate) { $AvrdudeConfig = $candidate }
}
$baseArgs = @('-p','m328pb','-c','arduino','-P',$Port,'-b',$Baud.ToString())
if (-not [string]::IsNullOrWhiteSpace($AvrdudeConfig)) {
    $baseArgs = @('-C',[IO.Path]::GetFullPath($AvrdudeConfig)) + $baseArgs
}
Set-Location -LiteralPath $taskDir
if (-not ('AvrProcessRunner' -as [type])) { Add-Type -LiteralPath (Join-Path $sourceDir 'AvrProcessRunner.cs') }
$script:checks = [Collections.Generic.List[object]]::new()
$script:serialPort = $null

function Add-Check([string]$Name) {
    $script:checks.Add([ordered]@{name=$Name; result='PASS'})
    [Console]::WriteLine('PASS ' + $Name)
}
function Assert-Bytes([byte[]]$Actual, [byte[]]$Expected, [string]$Name) {
    if ($Actual.Length -ne $Expected.Length) { throw "$Name length mismatch." }
    for ($index=0; $index -lt $Expected.Length; $index++) {
        if ($Actual[$index] -ne $Expected[$index]) { throw "$Name data mismatch." }
    }
}
function Assert-Files([string]$Actual, [string]$Expected) {
    if ((Get-FileHash -LiteralPath $Actual).Hash -ne (Get-FileHash -LiteralPath $Expected).Hash) {
        throw "File comparison failed for $Actual."
    }
}
function Invoke-Avr([string[]]$NativeArgs) {
    $code = [AvrProcessRunner]::Run($avrExe, $taskDir, [string[]]($baseArgs + $NativeArgs))
    if ($code -ne 0) { throw "AVRDUDE exited with code $code." }
}
function Open-Serial {
    $script:serialPort = [IO.Ports.SerialPort]::new($Port,$Baud,[IO.Ports.Parity]::None,8,[IO.Ports.StopBits]::One)
    $script:serialPort.DtrEnable = $false
    $script:serialPort.RtsEnable = $false
    $script:serialPort.ReadTimeout = 1500
    $script:serialPort.WriteTimeout = 1500
    $script:serialPort.Open()
}
function Close-Serial {
    if ($null -ne $script:serialPort) { $script:serialPort.Dispose(); $script:serialPort=$null }
}
function Read-Raw([int]$Count) {
    $bytes = [byte[]]::new($Count)
    $offset = 0
    while ($offset -lt $Count) {
        $read = $script:serialPort.Read($bytes,$offset,$Count-$offset)
        if ($read -le 0) { throw 'Serial port closed before the reply completed.' }
        $offset += $read
    }
    return ,$bytes
}
function Send-Expect([byte[]]$Packet, [byte[]]$Expected, [string]$Name) {
    $script:serialPort.Write($Packet,0,$Packet.Length)
    $reply = Read-Raw $Expected.Length
    Assert-Bytes $reply $Expected $Name
}
function Enter-Boot {
    $script:serialPort.DtrEnable=$false
    $script:serialPort.RtsEnable=$false
    Start-Sleep -Milliseconds 250
    $script:serialPort.DtrEnable=$true
    $script:serialPort.DtrEnable=$false
    Start-Sleep -Milliseconds 100
    $script:serialPort.DiscardInBuffer()
    Send-Expect ([byte[]]@(0x30,0x20)) ([byte[]]@(0x14,0x10)) 'GET_SYNC after external reset'
}
function Load-Address([int]$ByteAddress) {
    if (($ByteAddress -band 1) -ne 0) { throw 'Word addressing requires an even start address.' }
    Send-Expect ([byte[]]@(0x55,(($ByteAddress -shr 1)-band 0xFF),(($ByteAddress -shr 9)-band 0xFF),0x20)) ([byte[]]@(0x14,0x10)) 'LOAD_ADDRESS'
}
function Read-Memory([int]$Address,[int]$Count,[byte]$Type,[byte[]]$Expected,[switch]$LegacyZero) {
    Load-Address $Address
    $high=($Count-shr 8)-band 0xFF; $low=$Count-band 0xFF
    if ($LegacyZero) { $high=0; $low=0 }
    Send-Expect ([byte[]]@(0x74,$high,$low,$Type,0x20)) ([byte[]](@(0x14)+$Expected+@(0x10))) 'READ_PAGE'
}
function Write-Memory([int]$Address,[byte]$Type,[byte[]]$Payload) {
    Load-Address $Address
    $packet=[byte[]](@(0x64,(($Payload.Length-shr 8)-band 0xFF),($Payload.Length-band 0xFF),$Type)+$Payload+@(0x20))
    Send-Expect $packet ([byte[]]@(0x14,0x10)) 'PROG_PAGE'
}
function Capture-Output([int]$Milliseconds) {
    $text=[Text.StringBuilder]::new()
    $timer=[Diagnostics.Stopwatch]::StartNew()
    while ($timer.ElapsedMilliseconds -lt $Milliseconds) {
        [void]$text.Append($script:serialPort.ReadExisting())
        Start-Sleep -Milliseconds 20
    }
    return $text.ToString()
}
function Assert-AppStarted([string]$Text,[string]$Name) {
    if (-not $Text.Contains('BOOT_ACCESS_DIAGNOSTICS_V1') -or $Text.Contains([char]0x10)) {
        throw "$Name did not reject the packet and restart the diagnostic application as expected."
    }
}
function Reject-And-Restart([byte[]]$Packet,[string]$Name,[int]$WaitMs=900) {
    $script:serialPort.Write($Packet,0,$Packet.Length)
    $capture=Capture-Output $WaitMs
    Assert-AppStarted $capture $Name
    Add-Check $Name
    Enter-Boot
}
function Crc16([byte[]]$Data) {
    $crc=0xFFFF
    foreach ($value in $Data) {
        $crc=$crc-bxor([int]$value-shl 8)
        for($bit=0;$bit-lt 8;$bit++) {
            if(($crc-band 0x8000)-ne 0){$crc=(($crc-shl 1)-bxor 0x1021)-band 0xFFFF}
            else{$crc=($crc-shl 1)-band 0xFFFF}
        }
    }
    return $crc.ToString('X4')
}
function Probe-Ready([int]$TimeoutMs=1500,[switch]$AfterArmed) {
    $text=[Text.StringBuilder]::new();$timer=[Diagnostics.Stopwatch]::StartNew()
    while($timer.ElapsedMilliseconds-lt $TimeoutMs){
        [void]$text.Append($script:serialPort.ReadExisting())
        $received=$text.ToString()
        if($AfterArmed){
            $armed=$received.IndexOf('WDT_ARMED')
            if($armed-lt 0){Start-Sleep -Milliseconds 5;continue}
            $received=$received.Substring($armed)
        }
        $match=[regex]::Match($received,'APP_READY reason=([0-9A-F]{2}) r1=([0-9A-F]{2}) mcusr=([0-9A-F]{2}) wdt=([0-9A-F]{2}) entry=([0-9A-F]{16}) ticks=([0-9A-F]{4})')
        if($match.Success){
            return [pscustomobject]@{reason=[Convert]::ToInt32($match.Groups[1].Value,16);
                r1=[Convert]::ToInt32($match.Groups[2].Value,16);mcusr=[Convert]::ToInt32($match.Groups[3].Value,16);
                wdt=[Convert]::ToInt32($match.Groups[4].Value,16);entry=[Convert]::FromHexString($match.Groups[5].Value);
                ticks=[Convert]::ToInt32($match.Groups[6].Value,16);elapsed_ms=$timer.ElapsedMilliseconds}
        }
        Start-Sleep -Milliseconds 5
    }
    throw 'The watchdog probe did not report a complete application startup.'
}
function Assert-Probe($Ready,[int]$ReasonMask) {
    if(($Ready.reason-band $ReasonMask)-eq 0-or $Ready.r1-ne 0-or $Ready.mcusr-ne 0-or ($Ready.wdt-band 0x48)-ne 0){
        throw ('Unexpected boot state: reason={0:X2}, r1={1:X2}, MCUSR={2:X2}, WDTCSR={3:X2}.' -f $Ready.reason,$Ready.r1,$Ready.mcusr,$Ready.wdt)
    }
}
function Wait-PortState([bool]$Present) {
    $timer=[Diagnostics.Stopwatch]::StartNew()
    while($timer.ElapsedMilliseconds-lt 120000){
        if(Test-Path -LiteralPath 'cancel-cold-start.txt'){throw 'Cold-start test cancelled; restoring the original application.'}
        $found=@([IO.Ports.SerialPort]::GetPortNames())-contains $Port
        if($found-eq $Present){return}
        Start-Sleep -Milliseconds 100
    }
    throw 'Timed out waiting for the Nano USB port to disappear or return.'
}

function Tool-Path([string]$Path) {
    $full = [IO.Path]::GetFullPath($Path)
    if (-not $UseWslToolchain) { return $full }
    $converted = & wsl.exe --exec wslpath -a -u $full
    if ($LASTEXITCODE -ne 0 -or @($converted).Count -ne 1) { throw 'WSL path conversion failed.' }
    return [string]$converted
}
function Invoke-BuildTool([string]$Name,[string[]]$ToolArgs) {
    if ($UseWslToolchain) { & wsl.exe --exec $Name @ToolArgs }
    else { & $Name @ToolArgs }
    if ($LASTEXITCODE -ne 0) { throw "$Name failed." }
}
if (-not $restoreOnly) {
    foreach ($probe in @('diagnostics','watchdog_probe')) {
        $source = Tool-Path (Join-Path $sourceDir ($probe+'.c'))
        $elf = Tool-Path (Join-Path $taskDir ($probe+'.elf'))
        $bin = Tool-Path (Join-Path $taskDir ($probe+'.bin'))
        Invoke-BuildTool 'avr-gcc' @('-mmcu=atmega328p','-DF_CPU=16000000UL','-DDIAGNOSTICS_STANDALONE',
            '-Os','-std=gnu11','-ffunction-sections','-fdata-sections',$source,'-Wl,--gc-sections','-o',$elf)
        Invoke-BuildTool 'avr-objcopy' @('-O','binary','-R','.eeprom',$elf,$bin)
        Invoke-BuildTool 'avr-objdump' @('-d',$elf) | Set-Content -LiteralPath ($probe+'.disassembly.txt') -Encoding utf8
        if ([regex]::IsMatch([IO.File]::ReadAllText((Join-Path $taskDir ($probe+'.disassembly.txt'))),'(?m)^\s*[0-9a-f]+:.*\bspm\b')) {
            throw 'A diagnostic probe contains an SPM instruction.'
        }
    }
    Invoke-Avr @('-n','-D','-A','-U','signature:r:before-signature.bin:r',
        '-U','lfuse:r:before-lfuse.bin:r','-U','hfuse:r:before-hfuse.bin:r','-U','efuse:r:before-efuse.bin:r',
        '-U','lock:r:before-lock.bin:r','-U','flash:r:before-flash.bin:r','-U','eeprom:r:before-eeprom.bin:r')
    Invoke-Avr @('-n','-D','-A','-U','flash:v:before-flash.bin:r','-U','eeprom:v:before-eeprom.bin:r',
        '-U','signature:v:before-signature.bin:r','-U','lfuse:v:before-lfuse.bin:r','-U','hfuse:v:before-hfuse.bin:r',
        '-U','efuse:v:before-efuse.bin:r','-U','lock:v:before-lock.bin:r')
    Set-Content -LiteralPath 'backup-verified.txt' -Value 'Complete current-board backups independently verified before any write.' -Encoding utf8
}
if (-not (Test-Path -LiteralPath 'backup-verified.txt')) { throw 'Verified current-board backups are required.' }
$beforeFlash=[IO.File]::ReadAllBytes((Join-Path $taskDir 'before-flash.bin'))
$beforeEeprom=[IO.File]::ReadAllBytes((Join-Path $taskDir 'before-eeprom.bin'))
$diagnostic=[IO.File]::ReadAllBytes((Join-Path $taskDir 'diagnostics.bin'))
if($beforeFlash.Length-ne 32768-or $beforeEeprom.Length-ne 1024-or $diagnostic.Length-le 0-or $diagnostic.Length-gt 0x7E00){throw 'Unexpected image size.'}
foreach($entry in @(@('signature','1E9516'),@('lfuse','FF'),@('hfuse','DE'),@('efuse','FD'),@('lock','CF'))){
    $value=[Convert]::ToHexString([IO.File]::ReadAllBytes((Join-Path $taskDir ('before-'+$entry[0]+'.bin'))))
    if($value-ne $entry[1]){throw ('Unexpected '+$entry[0]+'.')}
}
$boot=[byte[]]::new(512); [Array]::Copy($beforeFlash,0x7E00,$boot,0,512)
$bootHash=[Convert]::ToHexString([Security.Cryptography.SHA256]::HashData($boot)).ToLowerInvariant()
if($bootHash-ne '149ebc0347815b42daaa20defce83342462306e9ddd3f15c63eeb59baafd70d1'){throw 'Installed bootloader differs from the verified build.'}
if([regex]::IsMatch([IO.File]::ReadAllText((Join-Path $taskDir 'diagnostics.disassembly.txt')),'(?m)^\s*[0-9a-f]+:.*\bspm\b')){throw 'Diagnostic contains an SPM instruction.'}
$restoreApp=[byte[]]::new(0x7E00); [Array]::Copy($beforeFlash,0,$restoreApp,0,$restoreApp.Length)
[IO.File]::WriteAllBytes((Join-Path $taskDir 'restore-application.bin'),$restoreApp)
$testApp=[byte[]]::new(0x7E00)
for($i=0;$i-lt $testApp.Length;$i++){$testApp[$i]=(($i*73)-bxor($i-shr 8)-bxor 0xA5)-band 0xFF}
[Array]::Copy($diagnostic,0,$testApp,0,$diagnostic.Length)
[IO.File]::WriteAllBytes((Join-Path $taskDir 'test-application.bin'),$testApp)
$testFlash=[byte[]]$beforeFlash.Clone();[Array]::Copy($testApp,0,$testFlash,0,$testApp.Length)
[IO.File]::WriteAllBytes((Join-Path $taskDir 'test-full-flash.bin'),$testFlash)
$testEeprom=[byte[]]::new(1024)
for($i=0;$i-lt 1024;$i++){$testEeprom[$i]=(($i*37)-bxor($i-shr 8)-bxor 0x5A)-band 0xFF}
[IO.File]::WriteAllBytes((Join-Path $taskDir 'test-eeprom.bin'),$testEeprom)
[ordered]@{port=$Port;baud=$Baud;signature='1E9516';boot_sha256=$bootHash;application_test_bytes=32256;eeprom_test_bytes=1024;diagnostic_bytes=$diagnostic.Length;restore_in_finally=$true;private_output=$taskDir;restore_only=$restoreOnly}|ConvertTo-Json|Write-Output
if(-not $Run){return}
if ($restoreOnly) {
    Invoke-Avr @('-n','-D','-A','-U','signature:v:before-signature.bin:r','-U','lfuse:v:before-lfuse.bin:r',
        '-U','hfuse:v:before-hfuse.bin:r','-U','efuse:v:before-efuse.bin:r','-U','lock:v:before-lock.bin:r')
    Invoke-Avr @('-D','-A','-U','flash:w:restore-application.bin:r','-U','eeprom:w:before-eeprom.bin:r')
    Invoke-Avr @('-n','-D','-A','-U','flash:v:before-flash.bin:r','-U','eeprom:v:before-eeprom.bin:r')
    [Console]::WriteLine('Original Flash and EEPROM restored and verified from the saved backup.')
    return
}

# Verify that this is still the saved board before allowing any write.
Invoke-Avr @('-n','-D','-A','-U','flash:v:before-flash.bin:r','-U','eeprom:v:before-eeprom.bin:r',
    '-U','signature:v:before-signature.bin:r','-U','lfuse:v:before-lfuse.bin:r','-U','hfuse:v:before-hfuse.bin:r',
    '-U','efuse:v:before-efuse.bin:r','-U','lock:v:before-lock.bin:r')
Add-Check 'Initial full Flash, EEPROM, signature, fuse and lock verification'
$failure=$null
$restorePassed=$false
$coldStartPassed=$false
$watchdogMaxMs=0
$watchdogCompleted=0
$dtrCompleted=0
$coldQueryReason=$null
try {
    # Only application Flash is written; the protected boot section is excluded.
    Invoke-Avr @('-D','-A','-U','flash:w:test-application.bin:r','-U','eeprom:w:test-eeprom.bin:r')
    Add-Check 'AVRDUDE write and automatic verify of all 32256 application bytes'
    Add-Check 'AVRDUDE write and automatic verify of all 1024 EEPROM bytes'

    Open-Serial
    $capture=Capture-Output 7200
    [IO.File]::WriteAllText((Join-Path $taskDir 'runtime-diagnostics.txt'),$capture)
    $frames=[regex]::Matches($capture,'BOOT_ACCESS_DIAGNOSTICS_V1\r?\n(.*?)END_BOOT_ACCESS_DIAGNOSTICS',[Text.RegularExpressions.RegexOptions]::Singleline)
    if($frames.Count-lt 3){throw 'Fewer than three runtime diagnostic frames received.'}
    $zeroCrc=Crc16 ([byte[]]::new(512))
    foreach($frame in $frames){
        $fields=@{}; foreach($line in ($frame.Groups[1].Value -split '\r?\n')){if($line.Contains('=')){$pair=$line.Split('=',2);$fields[$pair[0]]=$pair[1]}}
        foreach($entry in @(@('signature','1E9516'),@('lfuse','FF'),@('hfuse','DE'),@('efuse','FD'),@('lock_raw','CF'),
            @('lock_implemented','0F'),@('repeated_reads','STABLE'),@('boot_size_hex','0200'),@('boot_start_hex','7E00'),
            @('reset_target','BOOT'),@('app_lpm_boot','BLOCKED_BY_LOCK_BITS'),@('boot_spm_boot','BLOCKED_BY_LOCK_BITS'),
            @('boot_lpm_app','ALLOWED_BY_LOCK_BITS'),@('boot_spm_app','ALLOWED_BY_LOCK_BITS'),@('app_read_boot_crc16',$zeroCrc))){
            if($fields[$entry[0]]-ne $entry[1]){throw ('Runtime diagnostic mismatch: '+$entry[0])}
        }
    }
    Add-Check 'Application starts after AVRDUDE LEAVE_PROGMODE and prints at 115200 baud'
    Add-Check 'Three stable direct application reads of signature, fuses and lock bits'
    Add-Check 'Application boot reads blocked; boot write protection decoded as enabled'

    Enter-Boot
    Send-Expect ([byte[]]@(0x31,0x20)) ([byte[]](@(0x14)+[Text.Encoding]::ASCII.GetBytes('AVR STK')+@(0x10))) 'GET_SIGN_ON'
    Send-Expect ([byte[]]@(0x41,0x81,0x20)) ([byte[]]@(0x14,8,0x10)) 'Version major'
    Send-Expect ([byte[]]@(0x41,0x82,0x20)) ([byte[]]@(0x14,3,0x10)) 'Version minor'
    Send-Expect ([byte[]]@(0x75,0x20)) ([byte[]]@(0x14,0x1E,0x95,0x16,0x10)) 'Signature'
    Add-Check 'Raw STK500v1 sync, sign-on, version and ATmega328PB signature'
    Read-Memory 0 256 0x46 ([byte[]]$testApp[0..255])
    Read-Memory 0 256 0x46 ([byte[]]$testApp[0..255]) -LegacyZero
    Read-Memory 0x7D80 256 0x46 ([byte[]]$testFlash[0x7D80..0x7E7F])
    Read-Memory 0x100 256 0x45 ([byte[]]$testEeprom[0x100..0x1FF])
    Add-Check '256-byte Flash and EEPROM reads, legacy zero length and app/boot boundary'

    Send-Expect ([byte[]]@(0x74,0,1,0x58,0x20)) ([byte[]]@(0x14,0x11)) 'Unknown read memory'
    Send-Expect ([byte[]](@(0x64,0,8,0x58)+@(0,0,0,0,0,0,0,0)+@(0x20))) ([byte[]]@(0x14,0x11)) 'Unknown write memory'
    Send-Expect ([byte[]]@(0x30,0x20)) ([byte[]]@(0x14,0x10)) 'Sync after rejected memory types'
    Add-Check 'Unknown memory types return FAILED and keep protocol synchronized'

    for($length=1;$length-le 16;$length++){
        $payload=[byte[]]::new($length);for($i=0;$i-lt $length;$i++){$payload[$i]=0x70+$i}
        Write-Memory 0x3F0 0x45 $payload
        Read-Memory 0x3F0 $length 0x45 $payload
        [Array]::Copy($payload,0,$testEeprom,0x3F0,$length)
    }
    Add-Check 'EEPROM blocks of every size 1 through 16 at the upper boundary'
    Write-Memory 0x7D80 0x46 ([byte[]]@(0xA5,0x5A))
    $partial=[byte[]]::new(128);for($i=0;$i-lt 128;$i++){$partial[$i]=0xFF};$partial[0]=0xA5;$partial[1]=0x5A
    Read-Memory 0x7D80 128 0x46 $partial
    Write-Memory 0x7D80 0x46 ([byte[]]$testApp[0x7D80..0x7DFF])
    Add-Check 'Two-byte Flash write erases unspecified bytes; full last application page restored'

    Load-Address 0x7D80
    Reject-And-Restart ([byte[]](@(0x64,0,128,0x46)+[byte[]]$testApp[0x7D80..0x7DFF]+@(0))) 'Invalid Flash EOP rejected before page erase'
    Read-Memory 0x7D80 128 0x46 ([byte[]]$testApp[0x7D80..0x7DFF])
    Load-Address 0x3F0
    Reject-And-Restart ([byte[]](@(0x64,0,16,0x45)+@(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0)+@(0))) 'Invalid EEPROM EOP rejected before commit'
    Read-Memory 0x3F0 16 0x45 ([byte[]]$testEeprom[0x3F0..0x3FF])
    Load-Address 0x7E00
    # The rejected boot payload is IDENTICAL to the installed first boot page.
    Reject-And-Restart ([byte[]](@(0x64,0,128,0x46)+[byte[]]$boot[0..127]+@(0x20))) 'Boot-section write rejected; watchdog returns to application'
    Read-Memory 0x7E00 128 0x46 ([byte[]]$boot[0..127])
    Load-Address 0x7FFE
    Reject-And-Restart ([byte[]]@(0x74,0,4,0x46,0x20)) 'Flash read beyond physical end rejected'
    Load-Address 0x3FE
    Reject-And-Restart ([byte[]]@(0x74,0,4,0x45,0x20)) 'EEPROM read beyond physical end rejected'
    Reject-And-Restart ([byte[]]@(0x74,1,1,0x46,0x20)) 'Read length 257 rejected'
    Reject-And-Restart ([byte[]]@(0x55,0,0x80,0x20)) 'Overflowing word address rejected'
    Reject-And-Restart ([byte[]]@(0x30,0)) 'Invalid SYNC EOP triggers watchdog recovery'
    $script:serialPort.DiscardInBuffer()
    $idleCapture=Capture-Output 3400
    Assert-AppStarted $idleCapture 'Bootloader idle timeout'
    Add-Check 'One-second bootloader idle watchdog starts the application'

    Enter-Boot
    foreach($length in @(0,1,3,129,130,256)){
        Load-Address 0x7D80
        $packet=[byte[]]@(0x64,(($length-shr 8)-band 0xFF),($length-band 0xFF),0x46,0x20)
        Reject-And-Restart $packet ('Invalid Flash write length '+$length+' rejected before erase')
    }
    Load-Address 0x7D82
    Reject-And-Restart ([byte[]]@(0x64,0,2,0x46,0,0,0x20)) 'Unaligned Flash write rejected'
    Load-Address 0x3F0
    Reject-And-Restart ([byte[]]@(0x64,0,17,0x45,0x20)) 'EEPROM write length 17 rejected'
    Load-Address 0x3FE
    Reject-And-Restart ([byte[]]@(0x64,0,4,0x45,0,0,0,0,0x20)) 'EEPROM write beyond physical end rejected'
    Load-Address 0x7D80
    Reject-And-Restart ([byte[]](@(0x64,0,128,0x46)+[byte[]]$testApp[0x7D80..0x7DBF])) 'Incomplete Flash packet leaves the old page intact' 1800
    Read-Memory 0x7D80 128 0x46 ([byte[]]$testApp[0x7D80..0x7DFF])
    Load-Address 0x3F0
    Reject-And-Restart ([byte[]](@(0x64,0,16,0x45)+@(0,0,0,0,0,0,0,0))) 'Incomplete EEPROM packet leaves the old bytes intact' 1800
    Read-Memory 0x3F0 16 0x45 ([byte[]]$testEeprom[0x3F0..0x3FF])
    Send-Expect ([byte[]]@(0x56,0,0,0,0,0x20)) ([byte[]]@(0x14,0,0x10)) 'Unsupported UNIVERSAL read-only query'
    Add-Check 'Unsupported UNIVERSAL operation returns zero without programming fuses'
    Send-Expect ([byte[]]@(0x52,0x20)) ([byte[]]@(0x14,0x10)) 'Compatibility chip erase acknowledgement'
    Read-Memory 0 128 0x46 ([byte[]]$testApp[0..127])
    Add-Check 'Legacy chip erase acknowledgement preserves application Flash'
    Close-Serial

    [IO.File]::WriteAllBytes((Join-Path $taskDir 'test-eeprom-after-raw.bin'),$testEeprom)
    Invoke-Avr @('-n','-D','-A','-U','flash:v:test-full-flash.bin:r','-U','eeprom:v:test-eeprom-after-raw.bin:r',
        '-U','lfuse:v:before-lfuse.bin:r','-U','hfuse:v:before-hfuse.bin:r','-U','efuse:v:before-efuse.bin:r','-U','lock:v:before-lock.bin:r')
    Add-Check 'Full Flash and EEPROM still correct after all raw protocol cases; fuses and locks unchanged'

    $probe=[IO.File]::ReadAllBytes((Join-Path $taskDir 'watchdog_probe.bin'))
    if($probe.Length-le 0-or $probe.Length-gt 0x7D80){throw 'Unexpected watchdog probe size.'}
    $probeImage=[byte[]]::new([int]([Math]::Ceiling($probe.Length/128.0)*128))
    for($i=0;$i-lt $probeImage.Length;$i++){$probeImage[$i]=0xFF}
    [Array]::Copy($probe,0,$probeImage,0,$probe.Length)
    [IO.File]::WriteAllBytes((Join-Path $taskDir 'watchdog-probe-pages.bin'),$probeImage)
    Invoke-Avr @('-D','-A','-U','flash:w:watchdog-probe-pages.bin:r')
    Open-Serial
    $script:serialPort.Write('I')
    $ready=Probe-Ready;Assert-Probe $ready 8
    Add-Check 'Boot handoff clears r1 and MCUSR before the C runtime and disables the watchdog'
    $script:serialPort.DiscardInBuffer();$script:serialPort.Write('P')
    $pong=Capture-Output 100
    if(-not $pong.Contains('PONG')){throw 'Application UART receive/echo failed.'}
    Add-Check 'Application UART transmit and receive at 115200 baud'
    for($cycle=0;$cycle-lt 50;$cycle++){
        $script:serialPort.DiscardInBuffer();$script:serialPort.Write('W')
        $ready=Probe-Ready -AfterArmed;Assert-Probe $ready 8
        if($ready.elapsed_ms-ge 750){throw 'Watchdog reset unexpectedly waited in the bootloader.'}
        $watchdogMaxMs=[Math]::Max($watchdogMaxMs,$ready.elapsed_ms)
        $watchdogCompleted++
    }
    Add-Check '50 application-triggered 15ms watchdog resets restart the application without a reset loop'
    for($cycle=0;$cycle-lt 20;$cycle++){
        Enter-Boot
        Send-Expect ([byte[]]@(0x75,0x20)) ([byte[]]@(0x14,0x1E,0x95,0x16,0x10)) 'Signature during reset stress'
        Send-Expect ([byte[]]@(0x51,0x20)) ([byte[]]@(0x14,0x10)) 'LEAVE_PROGMODE during reset stress'
        $ready=Probe-Ready;Assert-Probe $ready 8
        $dtrCompleted++
    }
    Add-Check '20 consecutive DTR reset, sync, signature and LEAVE_PROGMODE cycles'
    Enter-Boot
    try{$script:serialPort.BreakState=$true;Start-Sleep -Milliseconds 20}
    finally{$script:serialPort.BreakState=$false}
    $ready=Probe-Ready 2500;Assert-Probe $ready 8
    Add-Check 'UART BREAK/framing-error input recovers through the watchdog'
    Close-Serial
    for($session=0;$session-lt 3;$session++){
        Invoke-Avr @('-n','-D','-A','-U','signature:v:before-signature.bin:r','-U','hfuse:v:before-hfuse.bin:r','-U','lock:v:before-lock.bin:r')
    }
    Add-Check 'Three consecutive real AVRDUDE sessions reconnect and verify the device'
    if($ColdStart){
        Open-Serial
        Enter-Boot
        Send-Expect ([byte[]]@(0x51,0x20)) ([byte[]]@(0x14,0x10)) 'Start the armed cold-start probe'
        $ready=Probe-Ready;Assert-Probe $ready 8
        $script:serialPort.DiscardInBuffer()
        $script:serialPort.Write('C')
        $ready=Probe-Ready;Assert-Probe $ready 8
        Assert-Bytes $ready.entry ([byte[]]::new(8)) 'Cleared autonomous-start evidence record'
        if($ready.ticks-ne 0){throw 'Autonomous-start uptime counter did not clear.'}
        # Keep this handle open until physical removal so closing the serial
        # driver cannot reset the armed application before power is removed.
        [Console]::WriteLine('READY_FOR_POWER_CYCLE: unplug every Nano power source, then reconnect only its USB cable.')
        Wait-PortState $false
        Close-Serial
        Wait-PortState $true
        Start-Sleep -Milliseconds 4000
        Open-Serial
        $script:serialPort.Write('I')
        $ready=Probe-Ready;Assert-Probe $ready 0x0F
        $coldQueryReason=$ready.reason
        $record=$ready.entry
        [ordered]@{query_reset_reason=$ready.reason;first_application_entry=[Convert]::ToHexString($record);autonomous_uptime_ticks=$ready.ticks;tick_ms=500} |
            ConvertTo-Json | Set-Content -LiteralPath 'cold-start-observation.json' -Encoding utf8
        Assert-Bytes ([byte[]]$record[0..3]) ([Text.Encoding]::ASCII.GetBytes('STAR')) 'Autonomous-start evidence tag'
        if($ready.ticks-lt 3-or ($record[4]-band 0x0F)-eq 0-or $record[5]-ne 0-or $record[6]-ne 0-or ($record[7]-band 0x48)-ne 0){
            throw 'Application startup before the host connection was not confirmed, or its handoff state was invalid.'
        }
        $coldStartPassed=$true
        Add-Check 'Physical power-off/on runs the application autonomously before the host opens the port'
        Close-Serial
    }
} catch {
    $failure=$_.Exception.Message
    [Console]::WriteLine('TEST FAILURE: '+$failure)
} finally {
    Close-Serial
    [Console]::WriteLine('Restoring the original complete application and EEPROM.')
    try {
        Invoke-Avr @('-D','-A','-U','flash:w:restore-application.bin:r','-U','eeprom:w:before-eeprom.bin:r')
        Invoke-Avr @('-n','-D','-A','-U','flash:r:restored-flash.bin:r','-U','eeprom:r:restored-eeprom.bin:r',
            '-U','signature:v:before-signature.bin:r','-U','lfuse:v:before-lfuse.bin:r','-U','hfuse:v:before-hfuse.bin:r',
            '-U','efuse:v:before-efuse.bin:r','-U','lock:v:before-lock.bin:r')
        Assert-Files 'restored-flash.bin' 'before-flash.bin'
        Assert-Files 'restored-eeprom.bin' 'before-eeprom.bin'
        $restorePassed=$true
        Add-Check 'Original complete Flash, EEPROM, signature, fuses and lock bits restored and verified'
    } finally {
        [ordered]@{port=$Port;baud=$Baud;mcu='ATmega328PB';boot_bytes=512;boot_sha256=$bootHash;
            checks=$script:checks;failure=$failure;original_state_restored=$restorePassed;
            cold_start_requested=[bool]$ColdStart;cold_start_passed=$coldStartPassed;cold_start_query_reason=$coldQueryReason;
            watchdog_cycles=$watchdogCompleted;dtr_cycles=$dtrCompleted;max_watchdog_restart_host_ms=$watchdogMaxMs;
            lfuse='FF';hfuse='DE';efuse='FD';lock='CF'} |
            ConvertTo-Json -Depth 6 | Set-Content -LiteralPath 'usb-test-result.json' -Encoding utf8
    }
}
if(-not $restorePassed){throw 'Original state restoration was not verified.'}
if($null-ne $failure){throw $failure}
[Console]::WriteLine('ALL REQUESTED USB HARDWARE TESTS PASSED; ORIGINAL DATA RESTORED.')
