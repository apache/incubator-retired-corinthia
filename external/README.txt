README.txt 1.0                         UTF-8

                       EXTERNAL DOWNLOADS AREA STRUCTURE
                       =================================

The Corinthia repository external/ area is a section of the repository used
for downloading external dependencies required in constructing Corinthia code.
These dependencies are required when constructing Corinthia components on and
for Microsoft Windows.

The downloads are created in an externals/download/ subdirectory.  This is
not part of the repository and is created in a local working copy whenever
needed.  The scripts in externals/ provide the necessary downloading and
organization of the external material.

The files directly in externals/ level provide documentation and scripts for
carrying out the download and extraction of material that Corinthia components
depend on for their compilation and executable operation.  These files are
part of the Corinthia source code repository and are maintained with it.

There are two uses for the externals/ source code,

  1. as maintained scripts for downloading the current dependencies into
     the download/ folder in a form that is used by build procedures where
     these dependencies matter,

  2. as templates for introducing other download folders that a project
     may require as part of its capture of external material that is
     needed on Microsoft Windows.



 1. PREREQUISITES FOR USING THE SCRIPTS
    -----------------------------------

To use these scripts, it is necessary to have an Internet Connection and be
on a Microsoft Windows platform.  Operation can be in a properly-connected
Virtual Machine guest having an installed Windows Operating System.  The
operating system can be either x86 or x64 based.

The procedures employ Microsoft Windows batch (.bat) files and rely on the
Windows Scripting Host (WSH) for JScript access to Windows utility functions.

The scripts can be executed using a Windows Shell of the user's choice.  They
have been tested using Windows cmd.exe, Windows PowerShell, MSYS2 (bash),
CygWin (bash) and JSoft Take Command (formerly 4NT and 4DOS).  They have been
operated on Windows 8.1 Pro and Windows 10 Technical Preview as of January,
2015.

 2. OPERATION
    ---------

In a console session, simply arrange to execute the script

    external/extract_downloads.bat

wherever it is located on the local system.  The other script files are used
as needed to complete the extraction operations.

The external/download/ folder will be created if needed.  Any external package
that is not yet downloaded will be fetched from the Internet.  Already-
downloaded packages will not be downloaded again.  (To force a fresh set of
downloads, simply delete the download/ subfolder.)

The external packages will be stored in external/downloads/ and will remain
there for any future use so long as that folder is preserved.

If the set of external packages is complete, they will then be expanded into
download/include/, download/lib/, and download/bin/ subdirectories with the
files needed for compilation and execution of code that relies on the external
dependencies.

This extraction is done completely whenever extract_downloads is operated.  It
will not be performed if not all of the package downloads have succeeded.


 3. COMPILE-TIME DEPENDENCY ON THE EXTERNAL/DOWNLOAD CONTENT
    --------------------------------------------------------

The population of external/downloads/ is presumed in the root CMakeLists.txt
file in the 'if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")' entry.  That
definition and the current structure here must be kept synchronized.

These externals are for builds targetted to Win32 x86 runtime only.  Externals
needed for producing x64 executables require a different arrangement.


 4. EXTERNAL FILES TO BE INCLUDED WITH CORINTHIA SOFTWARE ON WINDOWS
    ----------------------------------------------------------------

The external/download/ folder is populated with include/, lib/, and bin/
subfolders that carry extracted material that Corinthia code depends on when
compiled for Windows.

The compiled executables will need to be installed in folders that also have
copies of DLLs from external/download/bin/

  SDL2.dll
  SDL2_image.dll
  iconv.dll
  libjpeg-9.dll
  libpng16-16.dll
  libtiff-5.dll
  libwebp-4.dll
  libxml2.dll
  zlib1.dll

The license files in bin/ should be carried along with those DLL files.

Not every Corinthia component requires all of these DLLs.  For finer details,
consult information on the individual component.


 5. MAINTENANCE AND CUSTOMIZATION PROCEDURES
    ----------------------------------------

    [TBD]

                       *** end of README.txt ***
