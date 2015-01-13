README.txt 1.1.1                     UTF-8

                        EXTERNAL DOWNLOADS SETUP AND USE
                        ================================

The Corinthia repository external/ area is a section of the repository used
for downloading external dependencies required in constructing Corinthia code.
These dependencies are required when constructing Corinthia components on and
for Microsoft Windows.

The downloads are created in an externals\download\ subdirectory.  This is
not part of the repository and is created in a local working copy whenever
needed.  The scripts in externals\ provide the necessary downloading and
organization of the external material.

The files directly in externals\ level provide documentation and scripts for
carrying out the download and extraction of material that Corinthia components
depend on for their compilation and executable operation.  These files are
part of the Corinthia source code repository and are maintained with it.

There are two uses for the externals\ source code,

  1. as maintained scripts for downloading the current dependencies into
     the download/ folder in a form that is used by build procedures where
     these dependencies matter,

  2. as templates for introducing other download folders that a project
     may require as part of its capture of external material that is
     needed on Microsoft Windows.


   CONTENT
     1. Prerequisites for Using the Scripts
     2. Operation
     3. Compile-Time Dependency on the External/Download Content
     4. External Files to be Included with Corinthia Software on Windows
     5. Maintenance and Customization Procedures
     6. Caveats

      - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

     Licensed to the Apache Software Foundation (ASF) under one
      or more contributor license agreements.  See the NOTICE file
      distributed with this work for additional information
      regarding copyright ownership.  The ASF licenses this file
      to you under the Apache License, Version 2.0 (the
      "License"); you may not use this file except in compliance
      with the License.  You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

      Unless required by applicable law or agreed to in writing,
      software distributed under the License is distributed on an
      "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
      KIND, either express or implied.  See the License for the
      specific language governing permissions and limitations
      under the License.

      - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


 1. PREREQUISITES FOR USING THE SCRIPTS
    -----------------------------------

To use these scripts, it is necessary to have an Internet Connection and be
on a Microsoft Windows platform.  Operation can be in a properly-connected
Virtual Machine guest having an installed Windows Operating System.  The
operating system can be either x86 or x64 based.

The procedures employ Microsoft Windows batch (.bat) files and rely on the
Windows Scripting Host (WSH) for JScript access to Windows utility functions.
The Windows Command Extensions must be in effect.  (These are usually set by
default and Visual Studio scripts depend on them also.)

The scripts can be executed using a console shell of the user's choice.  They
have been tested using Windows cmd.exe, Windows PowerShell, MSYS2 (bash),
CygWin (bash) and JSoft Take Command (formerly 4NT and 4DOS).  They have been
operated on Windows XP, Windows 8.1 Pro x64 and Windows 10 x64 Technical
Preview as of January, 2015.


 2. OPERATION
    ---------

In a console session, simply arrange to execute the script

    external\extract_downloads.bat

wherever it is located on the local system.  The other script files are used
as needed to complete the extraction operations.  The script will operate
relative to its own location, not where it is executed from.  That is,
the downloads\ sub-folder is always created in the same folder that holds
extract_downloads.bat and the other scripts.

The external\download\ folder will be created if needed.  Any external package
that is not yet downloaded will be fetched from the Internet.  Already-
downloaded packages will not be downloaded again.  (To force a fresh set of
downloads, simply delete the download\ subfolder.)

The external packages will be stored in external\downloads\ and will remain
there for any future use so long as that folder is preserved.

If the set of external packages is complete, they will then be expanded into
download\include\, download\lib\, and download\bin\ subdirectories with the
files needed for compilation and execution of code that relies on the external
dependencies.

This extraction is done completely each time extract_downloads is operated.
Extraction is not performed if any of the package downloads are missing.


 3. COMPILE-TIME DEPENDENCY ON THE EXTERNAL/DOWNLOAD CONTENT
    --------------------------------------------------------

The population of external\downloads\ is presumed in the root CMakeLists.txt
file in the 'if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")' entry.  That
definition and the current structure here must be kept synchronized.

These externals are for builds targeted to Win32 x86 runtime only.  Externals
needed for producing x64 executables require a different arrangement.


 4. EXTERNAL FILES TO BE INCLUDED WITH CORINTHIA SOFTWARE ON WINDOWS
    ----------------------------------------------------------------

The external\download\ folder is populated with include\, lib\, and bin\
subfolders that carry extracted material that Corinthia code depends on when
compiled for Windows.

The compiled executables will need to be installed in folders that also have
copies of DLLs from external\download\bin\

  iconv.dll
  libjpeg-9.dll
  libpng16-16.dll
  libtiff-5.dll
  libwebp-4.dll
  libxml2.dll
  SDL2.dll
  SDL2_image.dll
  zlib1.dll

The license files in bin\ are to be carried along with the DLL files they
apply to.

Not every Corinthia component requires all of these DLLs.  For finer details,
consult information on the individual component.


 5. MAINTENANCE AND CUSTOMIZATION PROCEDURES
    ----------------------------------------

    The file maintenance.txt provides more information on how to perform
    basic maintenance on these procedures.

    The file customize.txt provides more information on adaptation of
    the procedures for use in a differen location and download situation.

 6. CAVEATS
    -------

    The Windows Command Extensions must be enabled for the .bat scripts to
    work properly.

    The Windows file system is case insensitive.  Any two file names and URLs
    that differ only in case will collide.  There are also more restrictions
    on special characters.  E.g., ":" may not be used in file names and "\"
    is the path separator (not "/" which is also restricted).

    The scripts can be unsuccessful without any failure indication.  It is
    necessary to use troubleshooting procedures to confirm that a download
    is failing or that an extraction is not producing the expected content.
    It is possible for a download to fail with a delivered but incorrect
    file.


REVISIONS

 1.1.1 2015-01-12-19:24 Stray word removed from the list of DLLs in Section 4
 1.1.0 2015-01-08-21:20 Updated Draft with complete coverage
 1.0.0 2015-01-08-15:21 Initial Draft replacement of the original README.txt


                       *** end of README.txt ***
