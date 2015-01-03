@echo off
rem fetch_downloads.bat 1.00              UTF-8
rem   FETCH EXTERNAL ARCHIVES FOR NATIVE WINDOWS BUILDS OF CORINTHIA

MKDIR "%~dp0download" >nul 2>&1
rem Make download directory at same location as this script.
rem Be silent even if the directory already exists.  No worries.

rem  MAINTAIN THIS LIST MANUALLY.  MATCH UP IN EXTRACT_DOWNLOADS.BAT
CALL :FETCH "http://zlib.net/zlib128-dll.zip"
CALL :FETCH "http://xmlsoft.org/sources/win32/iconv-1.9.2.win32.zip"
CALL :FETCH "http://xmlsoft.org/sources/win32/libxml2-2.7.8.win32.zip"
CALL :FETCH "https://www.libsdl.org/release/SDL2-devel-2.0.3-VC.zip"
CALL :FETCH  "https://www.libsdl.org/projects/SDL_image/release/SDL2_image-devel-2.0.0-VC.zip"
EXIT /B 0

:FETCH
IF EXIST "%~dp0download\%~n1%~x1" EXIT /B 0
rem do not download an archive that is already present.
Cscript /nologo "%~dp0wget-win.js" //B "%1" "%~dp0download\%~n1%~x1"
IF EXIST "%~dp0download\%~n1%~x1" ECHO:     %~n1%~x1% downloaded
EXIT /B 0

rem TODO
rem  * Might want to pass up and act on error codes from the individual
rem    :FETCH operations.
rem  * It might be handy to fetch these URLs from a file so that it can
rem    be kept maintained in one place, even though the win32 cases are
rem    unique to building for Windows.
rem  * Silent the warning on download folder already existing
rem XXX
rem  * wget-win.js does not do FTP.  So alternative locations have been
rem    used for iconv and libmxl2.

rem 1.00 2015-01-02-16:51 Complete with mating to extract_downloads.bat
rem      The tested version is adjusted to list successful downloads
rem      and not any that are not needed or fail.

rem               *** end of fetch_downloads.bat ***
