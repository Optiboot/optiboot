@echo off
SET DEBUG=REM

REM   Batch file to build Optiboot by searching around for the right tools, and then
REM   setting the path and invoking them with the specified arguments.
REM   Based on https://github.com/WestfW/Arduino-avr-tools-install

REM Batch file must run in Delayed Evaluation mode.
%DEBUG% checking cmd mode
call :clearerrors
if "!x!" EQU "%x%" (

REM Things are good; try to set the paths and run the compile

REM --------------------------------------------------------
   call :findArduino
   %DEBUG% Using make %*
   make %*
   exit /b 0
REM --------------------------------------------------------

)
REM We have to run in /V mode to get late evaluation of variables inside loops
%DEBUG% re-running in delayed eval mode
cmd.exe /V /C "%0" %*
exit /b 0



REM ======================================================================

:findArduino
REM Here is most of the work of the script.
REM Look through the various places where an Arduino install is likely to exist,
REM  make sure that we can find the avr-gcc binaries that should be part of that
REM  install, print out the version, and ask the user if that's what they want.

REM Look for existing avr-gcc in path.
%DEBUG% Looking for avr-gcc in current path
call :clearerrors
call :which avr-gcc.exe
IF "%gotwhich%" NEQ "" (
  @ECHO avr-gcc already installed at %gotwhich%
  exit /b 123
) ELSE (
  @ECHO No avr-gcc in current path
)

REM look for Arduino install.
set picked=n
set printed=n
call :scandir "%ProgramFiles%"
if ERRORLEVEL 1 exit /b 1
if "%picked%" NEQ "y" call :scandir "%ProgramFiles(x86)%"
if ERRORLEVEL 1 exit /b 1
if "%picked%" NEQ "y" call :scandir "C:\bin"
if ERRORLEVEL 1 exit /b 1
if "%picked%" EQU "y" goto gotdir
@echo.
  REM try some of the more unlikely places that Arduino might live.

:noarduino
  @echo Can't find Arduino
  exit /b 1


REM Given the name of a program-containing directory (like "\bin"),
REM  See if it looks like there are any "Arduino*" directories there,
REM  and prompt the user to ask whether that's the one we want to use
REM  for avr-gcc.  If so, set variables saying we picked one.
:scandir
  %DEBUG% ScanDir %1
  SET root=%~1
  if NOT EXIST "%root%" exit /b 0
  if NOT EXIST "%root%\Arduino*" exit /b 0
  FOR /F "tokens=*" %%f IN ('dir /b /x "%root%\Arduino*"') DO (
    SET prg=!root!\%%f
    if exist "!prg!\Hardware\tools\avr\bin\avr-gcc.exe" (
       if "%printed%" NEQ "Y" (
          echo At least one Arduino install found.
          echo.
          set printed=Y
       )
       @echo Looks like !prg! has version 
       call :gccversion "!prg!\Hardware\tools\avr\bin\avr-gcc.exe"
       SET aroot=!prg!
       set picked=y
       exit /b 0
    ) else %DEBUG% No Arduino exe in expected spot !prg!
  )
  exit /b 0


REM prompt for arduino install location.
@echo ****WHY DID WE GET HERE****
@echo asking if this is OK
SET /P confirm="Use %prg% [y/n]>"
if "%confirm%" NEQ "y" exit /b 0


:gotdir
REM figure out arduino install version.
%DEBUG% gotdir has aroot = !aroot!
IF EXIST "!aroot!\hardware\tools\avr\bin\avr-gcc.exe" (
  set bin=!aroot!\hardware\tools\avr\bin
  set etc=!aroot!\hardware\tools\avr\etc
) else (
  @echo Cant find a bin directory in !aroot!
  exit /b 963
)

%DEBUG% Checking for utils at %prg%\hardware\tools\avr\utils\bin\
IF EXIST "!aroot!\hardware\tools\avr\utils\bin\make.exe" (
  REM See if we have make and etc as well (from Arduino 1.0.x/WinAVR)
  %DEBUG% Found make.exe
  set utils=!aroot!\hardware\tools\avr\utils\bin
) else (
  IF EXIST "C:\Program Files (X86)\GnuWin32" (
    set utils="C:\Program Files (X86)\GnuWin32\bin"
  )
  IF EXIST "C:\Program Files\GnuWin32" (
    set utils="C:\Program Files\GnuWin32\bin"
  )
)


REM find bin directory.
REM create tentative path for binaries and put it in our tmp batch file
%DEBUG% Setting paths to include bin and etc
REM
REM setx will set a permanent path in the registry, but it won't take effect
REM until the next invocation of cmd.exe
REM setx PATH %bin%;%etc%
REM
%DEBUG% adding arduin bin and etc to PATH %bin%;%etc%
call :shorten "%bin%"
PATH %PATH%;!shortout!
call :shorten "%etc%"
PATH %PATH%;!shortout!

%DEBUG% Have utils = %utils%

IF %utils% NEQ "" (
   REM Check for make already installed
   %DEBUG% Have utils; checking whether make is already installed.
   call :which make.exe
   if "%gotwhich%" EQU "" (
      echo Found Make at %utils%
      call :shorten %utils%
      PATH %PATH%;!shortout!
   )
)
call :clearerrors
exit /b 0
goto eof


REM ----------------------------------------------------------------------
REM          Subroutines
REM ----------------------------------------------------------------------

:which
    %DEBUG% which %1
    SET gotwhich=
    for %%i in (%1) do set fullspec=%%~$PATH:i
    if not "x!fullspec!"=="x" (
       %DEBUG% !fullspec!
       set gotwhich=!fullspec!    
    )
    %DEBUG% End which %gotwhich%
    goto eof


REM ----------------------------------------------------------------------

:gccversion
   REM This implements "gcc --version | head -1" - show the first line
   if EXIST %1 (
     FOR /F "delims=*" %%l in ('%1 --version') DO (
       @echo %%l
       REM return after we've output one line
       exit /b 0
     )
   )
   exit /b 0


REM ----------------------------------------------------------------------

:avrdudeversion
   SETLOCAL ENABLEDELAYEDEXPANSION
   REM This implements "gcc --version | head -4" - show the first line
   set /a count=4
   FOR /F "delims=*" %%l in ('"avrdude -v 2>&1"') DO (
     @echo %%l
     set /a count=!count!-1
     if !count! equ 0 (
        ENDLOCAL
        exit /b 0
     )
   )
   exit /b 0
goto :eof


REM ----------------------------------------------------------------------

REM Clear the ERRORLEVEL to 0, if it happened to be set
:clearerrors
   exit /b 0


REM ----------------------------------------------------------------------

REM Shorten a pathname to 8.3 format filenames.

:shorten
  set shortout=%~s1
  exit /b 0

:eof
