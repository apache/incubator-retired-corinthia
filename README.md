# About Socrates

Socrates is a library for converting between different word processing file
formats. Currently it supports .docx (part of the OOXML specification), HTML,
and LaTeX (export-only). Socrates also contain convinience executables.

[details](https://github.com/uxproductivity/Socrates/wiki/Incubator-proposal#abstract)

# License

Socrates is licensed under the Apache License version 2.0; see
LICENSE.txt for details.

# What the library can do

1. Create new HTML files from a .docx source
2. Create new .docx files from a HTML source
3. Update existing .docx files based on a modified HTML file produced in (1)
4. Convert .docx or HTML files to LaTeX
5. Provide access to document structure, in terms of a DOM-like API for
   manipulating XML trees, and an object model for working with CSS stylesheets

# Components

There are three major components, in their respective directories:

* `DocFormats` - the library itself
* `dfutil` - a driver program used for running [...]
* automated tests (located in the tests directory)

Run dfutil without any command-line arguments to see a list of operations. Here
is an example of converting a .docx file to HTML, modifying it, and then updating
the original .docx. Note that it is important, due to how internal mapping works,
that the .docx file being written is the same file as the original; using a new
file won't work.

    dfutil filename.docx filename.html
    vi filename.html # Make some changes
    dfutil filename.html filename.docx

If you examine the convertFile function in `dfutil/Commands.c`, you will see the
main entry points to perform these conversions, which you can call from your own
program.

# Current Status
DocFormats has been shipping as part of UX Write on the iOS app store since
February 2013.

[detailed status](https://github.com/uxproductivity/Socrates/wiki/Incubator-proposal#current-status)

# Platforms and dependencies

Socrates builds and runs on iOS, OS X, and Linux. Windows support is in the works.

To build DocFormats, you will need to have the following installed:

* [CMake](http://www.cmake.org)
* [libxml2](http://xmlsoft.org)
* [libiconv](https://www.gnu.org/software/libiconv/)
* [zlib](http://www.zlib.net)

# Build instructions

Socrates currently build on Linux and OS X (mac).

For detailed instructions, please see

[build instructions](https://github.com/uxproductivity/Socrates/wiki/Build-instructions)

# Further information

please see [mail list](https://github.com/uxproductivity/Socrates/wiki/Talk-to-us) for contacts
