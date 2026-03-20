@echo off
setlocal EnableExtensions EnableDelayedExpansion

echo.
echo NOTE: Execute this script from outside the SimDSP folder
echo.

set argc=0
for %%x in (%*) do set /A argc+=1

if /I "%argc%" NEQ "3" (
  call :usage
  exit /b 1
)

set "RELNAME=%~1"
set "QT_HOME=%~2"
set "MINGW_PATH=%~3"
if "%RELNAME%"=="" set "RELNAME=dev"

set "PATH=%MINGW_PATH%\bin;%QT_HOME%\bin;%PATH%"
set "QMAKE_BIN=%QT_HOME%\bin\qmake.exe"
if not exist "%QMAKE_BIN%" set "QMAKE_BIN=%QT_HOME%\bin\qmake-qt5.exe"
if not exist "%QMAKE_BIN%" (
  echo ERROR: qmake not found in %QT_HOME%\bin
  exit /b 1
)

set "arch=AMD64"
if /I "%PROCESSOR_ARCHITECTURE%"=="x86" set "arch=i386"

set "script_folder=%~dp0"
set "current_dir=%cd%"
set "app_folder=%script_folder%..\"
set "sdcore_folder=%app_folder%src\sdcore"
set "build_folder=%app_folder%src\build"
set "include_folder=%app_folder%src\include"
set "gui_folder=%app_folder%src\gui"

cd /d "%sdcore_folder%"
echo Compiling SimDSP core...
"%QMAKE_BIN%" -config release
mingw32-make.exe
if errorlevel 1 exit /b 1

cd /d "%app_folder%"
echo Compiling SimDSP app...
"%QMAKE_BIN%" -config release
mingw32-make.exe
if errorlevel 1 exit /b 1

set "release_name=simdsp-%RELNAME%.windows.%arch%"
set "release_folder=%current_dir%\%release_name%"
if exist "%release_folder%" rmdir /s /q "%release_folder%"
mkdir "%release_folder%"
mkdir "%release_folder%\resources\dlls"
mkdir "%release_folder%\resources\include"
mkdir "%release_folder%\resources\examples"
mkdir "%release_folder%\resources\icons"

copy "%app_folder%scripts\simdsp.bat" "%release_folder%\" >nul
copy "%app_folder%README.md" "%release_folder%\" >nul
copy "%app_folder%LICENSE" "%release_folder%\" >nul
copy "%gui_folder%\resources\images\icons\simdsp.ico" "%release_folder%\resources\icons\" >nul
copy "%build_folder%\SimDSP.exe" "%release_folder%\resources\dlls\" >nul
if errorlevel 1 (
  echo ERROR: SimDSP.exe not found in %build_folder%
  exit /b 1
)

if exist "%build_folder%\sdcore.dll" copy "%build_folder%\sdcore.dll" "%release_folder%\resources\dlls\" >nul
if exist "%build_folder%\libsdcore.a" copy "%build_folder%\libsdcore.a" "%release_folder%\resources\dlls\" >nul
if exist "%app_folder%dependencies\rtaudio\libs\windows-64\librtaudio-6.dll" copy "%app_folder%dependencies\rtaudio\libs\windows-64\librtaudio-6.dll" "%release_folder%\resources\dlls\" >nul
if exist "%app_folder%dependencies\rtaudio\libs\windows-64\librtaudio.dll.a" copy "%app_folder%dependencies\rtaudio\libs\windows-64\librtaudio.dll.a" "%release_folder%\resources\dlls\" >nul
if exist "%app_folder%dependencies\rtaudio\libs\windows-64\librtaudio.a" copy "%app_folder%dependencies\rtaudio\libs\windows-64\librtaudio.a" "%release_folder%\resources\dlls\" >nul

copy "%app_folder%dependencies\matio\libmatio.dll" "%release_folder%\resources\dlls\" >nul
copy "%app_folder%dependencies\matio\hdf5.dll" "%release_folder%\resources\dlls\" >nul
copy "%app_folder%dependencies\matio\zlib.dll" "%release_folder%\resources\dlls\" >nul
copy "%app_folder%dependencies\fftw3\libfftw3-3.dll" "%release_folder%\resources\dlls\" >nul
copy "%app_folder%dependencies\fftw3\libfftw3f-3.dll" "%release_folder%\resources\dlls\" >nul
copy "%app_folder%dependencies\fftw3\libfftw3l-3.dll" "%release_folder%\resources\dlls\" >nul

copy "%include_folder%\simdsp.h" "%release_folder%\resources\include\" >nul
copy "%include_folder%\sdfunctions.h" "%release_folder%\resources\include\" >nul
xcopy /s /e /i "%app_folder%src\examples" "%release_folder%\resources\examples" >nul

cd /d "%release_folder%\resources\dlls"
windeployqt.exe --no-angle SimDSP.exe
if errorlevel 1 exit /b 1

if exist "%MINGW_PATH%\bin\objdump.exe" (
  call :copy_imports "%MINGW_PATH%\bin\objdump.exe" "%MINGW_PATH%\bin" "%release_folder%\resources\dlls"
)

mkdir "%release_folder%\MinGW"
xcopy /s /e /i "%MINGW_PATH%\bin" "%release_folder%\MinGW\bin" >nul
xcopy /s /e /i "%MINGW_PATH%\include" "%release_folder%\MinGW\include" >nul
if exist "%MINGW_PATH%\x86_64-w64-mingw32" xcopy /s /e /i "%MINGW_PATH%\x86_64-w64-mingw32" "%release_folder%\MinGW\x86_64-w64-mingw32" >nul
if exist "%MINGW_PATH%\lib\gcc" xcopy /s /e /i "%MINGW_PATH%\lib\gcc" "%release_folder%\MinGW\lib\gcc" >nul
if exist "%MINGW_PATH%\libexec\gcc" xcopy /s /e /i "%MINGW_PATH%\libexec\gcc" "%release_folder%\MinGW\libexec\gcc" >nul

if not exist "%release_folder%\resources\dlls\Qt5Core.dll" (
  echo ERROR: Qt5Core.dll missing from package
  exit /b 1
)

echo Package created: %release_folder%
exit /b 0

:copy_imports
set "OBJDUMP=%~1"
set "SRCBIN=%~2"
set "DSTBIN=%~3"

for %%F in ("%DSTBIN%\*.exe" "%DSTBIN%\*.dll") do (
  if exist "%%~F" (
    for /f "tokens=3" %%D in ('"%OBJDUMP%" -p "%%~F" ^| findstr /c:"DLL Name:"') do (
      if not exist "%DSTBIN%\%%D" (
        if exist "%SRCBIN%\%%D" copy "%SRCBIN%\%%D" "%DSTBIN%\" >nul
      )
    )
  )
)
exit /b 0

:usage
echo Usage : release.bat version qt_path mingw_path
exit /b 0
