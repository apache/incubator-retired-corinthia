@echo off
rem fetch_downloads.bat               UTF-8
rem   FETCH EXTERNAL ARCHIVES FOR NATIVE WINDOWS BUILDS OF CORINTHIA

mkdir %~dp0download
rem Make download directory at same location as this script.
rem It does not matter if the directory already exists.

rem  MAINTAIN THIS LIST MANUALLY
CALL :FETCH "http://zlib.net/zlib128-dll.zip"
CALL :FETCH "http://xmlsoft.org/sources/win32/iconv-1.9.2.win32.zip"
CALL :FETCH "http://xmlsoft.org/sources/win32/libxml2-2.7.8.win32.zip"
CALL :FETCH "https://www.libsdl.org/release/SDL2-devel-2.0.3-VC.zip"
CALL :FETCH  "https://www.libsdl.org/projects/SDL_image/release/SDL2_image-devel-2.0.0-VC.zip"
EXIT /B 0
rem TODO
rem  * Might want to pass up and act on error codes from the individual
rem    :FETCH operations.
rem  * It might be handy to fetch these URLs from a file so that it can
rem    be kept maintained in one place, even though the win32 cases are
rem    unique to building for Windows.
rem XXX
rem  * wget-win.js does not do FTP.  So alternative locations have been
rem    used for iconv and libmxl2.

:FETCH
IF EXIST "%~dp0download\%~n1%~x1" EXIT /B 0
rem do not download an archive that is already present.
ECHO "%~n1%~x1"
Cscript /nologo "%~dp0wget-win.js" //B "%1" "%~dp0download\%~n1%~x1"
EXIT /B 0

