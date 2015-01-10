@echo off
rem extract_downloads.bat 1.1.0       UTF-8
rem    EXTRACT THE EXTERNAL DOWNLOADS TO INCLUDE, LIB, AND BIN FOLDERS

rem Fetch downloads in case not done yet
CALL "%~dp0fetch_downloads.bat"

rem Determine if any of the packages are missing
SET extract_downloads=
CALL :CHECKON zlib128-dll.zip
CALL :CHECKON iconv-1.9.2.win32.zip
CALL :CHECKON libxml2-2.7.8.win32.zip
CALL :CHECKON SDL2-devel-2.0.3-VC.zip
CALL :CHECKON SDL2_image-devel-2.0.0-VC.zip

rem This procedure can't proceed unless all are present
IF NOT "%extract_downloads%" == "" GOTO :FAIL1

CALL :CLEAN

rem For extractions, use the zip name listed and the path, if any, from the
rem root of the zipped hierarchy to the level where include and libs are found

rem           File Name                       Top Path
CALL :SDL2x86 SDL2-devel-2.0.3-VC.zip         SDL2-2.0.3\
CALL :SDL2x86 SDL2_image-devel-2.0.0-VC.zip   SDL2_image-2.0.0\
CALL :ICONV   iconv-1.9.2.win32.zip           iconv-1.9.2.win32\
CALL :LIBXML2 libxml2-2.7.8.win32.zip         libxml2-2.7.8.win32\
CALL :ZLIB    zlib128-dll.zip                 ""

EXIT /B 0

rem MOST MAINTENANCE IS BY UPDATING THE FILENAMES AND TOP PATHS ABOVE.
rem    The extraction procedure do not require maintenance unless there is
rem    a Zip layout change or new extraction cases are needed.

rem CUSTOM EXTRACTION CASES FOR COLLECTING EXTERNAL INCLUDES, LIBS, AND BINS
rem    Each one of these cases below accepts two parameters
rem       %1 is the filename of the zip file, to be unzipped from
rem          %~dp0download\%1 with expansion to folder %~dp0download\T
rem       %2 is the path from T to the level at which parts like include
rem          and lib are to be found.  %2 is either empty or it is one or more
rem          path segments each ending with "\".

:SDL2x86
rem taking T\%2include and T\%2lib\*.lib across, with T\%2lib\*.dll to bin
rem move any license *.txt files to the bin also.
CALL :UNZIP %1
XCOPY "%~dp0download\T\%2include\*.*" "%~dp0download\include" /I /Q /Y >nul
XCOPY "%~dp0download\T\%2lib\x86\*.lib" "%~dp0download\lib" /I /Q /Y >nul
XCOPY "%~dp0download\T\%2lib\x86\*.dll" "%~dp0download\bin" /I /Q /Y >nul
XCOPY "%~dp0download\T\%2lib\x86\*.txt" "%~dp0download\bin" /I /Q /Y >nul 2>&1
EXIT /B 0

:ICONV
rem taking T\%2include, T\%2lib, and T\%2bin\*.dll across
CALL :UNZIP %1
XCOPY "%~dp0download\T\%2include\*.*" "%~dp0download\include" /I /Q /Y >nul
XCOPY "%~dp0download\T\%2lib\*.*" "%~dp0download\lib" /I /Q /Y >nul
XCOPY "%~dp0download\T\%2bin\*.dll" "%~dp0download\bin" /I /Q /Y >nul
EXIT /B 0

:LIBXML2
rem taking T\%2include\libxml\, T\%2lib, and T\%2bin\*.dll across
CALL :UNZIP %1
XCOPY "%~dp0download\T\%2include\libxml\*.*" "%~dp0download\include\libxml" /I /Q /Y >nul 2>&1
XCOPY "%~dp0download\T\%2lib\*.*" "%~dp0download\lib" /I /Q /Y >nul
XCOPY "%~dp0download\T\%2bin\*.dll" "%~dp0download\bin" /I /Q /Y >nul
EXIT /B 0

:ZLIB
rem taking all across from T\%2include and T\%2lib, with T\%2*.dll to bin
CALL :UNZIP %1
XCOPY "%~dp0download\T\%2include\*.*" "%~dp0download\include" /I /Q /Y >nul
XCOPY "%~dp0download\T\%2lib\*.*" "%~dp0download\lib" /I /Q /Y >nul
XCOPY "%~dp0download\T\%2*.dll" "%~dp0download\bin" /I /Q /Y >nul
EXIT /B 0

:UNZIP
rem EXTRACT ALL OF ZIP "%~dp0download\%1" TO "%~dp0download\T"
rem     "%~dp0download\T" is not deleted until needed again, leaving the
rem     last one for inspection when trouble-shooting.
RMDIR /S /Q "%~dp0download\T" >nul 2>&1
ECHO:     extracting %1
Cscript /nologo "%~dp0unzip-win.js" //B "%~dp0download\%1" "%~dp0download\T"
EXIT /B 0

:CHECKON
IF EXIST "%~dp0download\%1" EXIT /B 0
IF "%extract_downloads%" == "" ECHO:
ECHO: *** %1 UNAVAILABLE FOR EXTRACTION
SET extract_downloads=0
EXIT /B 2

:CLEAN
rem clean out any previous material and be quiet about it
RMDIR /S /Q "%~dp0download\include" >nul 2>&1
RMDIR /S /Q "%~dp0download\lib" >nul 2>&1
RMDIR /S /Q "%~dp0download\bin" >nul 2>&1
rem set up empty include, lib, and bin to receive fresh extractions
MKDIR "%~dp0download\include"
MKDIR "%~dp0download\lib"
MKDIR "%~dp0download\bin"
EXIT /B 0

:FAIL1
SET extract_downloads=
ECHO: *** EXTRACTION REQUIRES ALL OF THE DOWNLOADS TO BE AVAILABLE
ECHO: *** Ensure that the archives downloaded by fetch_downloads.bat and
ECHO: *** expected here are the same.  If the unavailable archives are
ECHO: *** no longer found at the URLs used in fetch_downloads.bat, find
ECHO: *** alternative locations for them.
ECHO: ***    No extractions have been performed.
ECHO:
EXIT /B 2

rem 1.1.0 2015-01-08-14:41 Extract SDL2 Licenses
rem       The license files are added to the bin\ extraction.
rem 1.01  2015-01-02-17:03 Silence warnings when removing non-existent
rem       directories
rem 1.00  2015-01-02-16:25 Complete Full-Functioning Externals Extraction
rem       Delivering the download\include, donwload\lib, and download\bin
rem       collections established for the current external downloads.

rem                 *** end of extract_downloads.bat ***
