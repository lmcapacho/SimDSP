@echo off

cls

echo.
echo NOTE: Execute this script from outside the SimDSP folder
echo.

set argc=0
for %%x in (%*) do Set /A argc+=1

IF /I "%argc%" NEQ "3" (
  call :usage
  echo.
  exit
)

set RELNAME=%1
set QT_HOME=%2
set MINGW_PATH=%3

echo.

reg Query "HKLM\Hardware\Description\System\CentralProcessor\0" | find /i "x86" > NUL && set arch_aux="i386" || set arch_aux="x86_64"

set script_folder=%~dp0%

echo script_folder %script_folder%
set current_dir=%cd%

IF %arch_aux%=="x86_64" (
  set arch=AMD64
) ELSE (
  set arch=i386
)

set app_folder=%script_folder%..\
set sdcore_folder=%app_folder%sdcore

echo.
echo appfolder %app_folder%
echo sdcorefolder %sdcore_folder%

echo.
cd %sdcore_folder%
echo Compiling SimDSP core...
%QT_HOME%\bin\qmake.exe -config release
%MINGW_PATH%\bin\mingw32-make.exe

echo.
cd %app_folder%
echo Compiling SimDSP app...
%QT_HOME%\bin\qmake -config release
%MINGW_PATH%\bin\mingw32-make.exe

echo.
set release_name=simdsp-%RELNAME%.windows.%arch%
set release_folder=%current_dir%\%release_name%
echo making release folder: %release_folder%
if not exist %release_folder% mkdir %release_folder%

echo.
echo copying release files
copy simdsp.bat %release_folder%
copy README.md %release_folder%
copy LICENSE %release_folder%

if not exist %release_folder%\icons mkdir %release_folder%\icons
copy resources\images\icons\simdsp.ico %release_folder%\icons
copy SimDSP.exe %release_folder%

echo.
cd %release_folder%
echo making include folder
if not exist include mkdir include

echo.
echo copying headers files
copy %sdcore_folder%\simdsp.h include
copy %sdcore_folder%\sdfunctions.h include

echo.
if not exist examples mkdir examples
echo copying examples
xcopy /s/e %app_folder%\examples examples

echo.
echo making libraries folder
if not exist dlls mkdir dlls
copy SimDSP.exe dlls

echo.
cd dlls
echo copying libraries
%QT_HOME%\bin\windeployqt.exe SimDSP.exe
del SimDSP.exe

echo.
echo copying sdcore libraries
copy %sdcore_folder%\build\sdcore.dll
copy %sdcore_folder%\build\libsdcore.a
copy %app_folder%\resources\dependencies\fftw3\libfftw3-3.dll

echo.
cd %release_folder%
if not exist resources mkdir resources
move SimDSP.exe dlls
move include resources
move dlls\platforms resources
move dlls resources
move examples resources
move icons resources

%script_folder%\Bat_To_Exe_Converter\Bat_To_Exe_Converter_x64.exe /bat simdsp.bat /exe SimDSP.exe /icon resources\icons\simdsp.ico
del simdsp.bat

if not exist MinGW mkdir MinGW
xcopy /s/e %MINGW_PATH% MinGW

echo done!

exit

:usage
echo Usage : release.bat version qt_path mingw_path
EXIT /B 0
