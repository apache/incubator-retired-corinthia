# About Corinthia

Corinthia is a library for converting between different word processing file
formats. Currently it supports .docx (part of the OOXML specification), HTML,
and LaTeX (export-only). Corinthia also contain convinience executables.

We are putting together a proposal to make this an Apache Incubator project. [See here for details](https://github.com/uxproductivity/Corinthia/wiki/Incubator-proposal#abstract)

For more information, contact any of the following people

- Peter Kelly <peter@uxproductivity.com>
- Louis Suárez-Potts <luispo@gmail.com>
- Jan Iverson <jani@apache.org>

Please note that the documentation for this project is in a very preliminary stage. We'll be improving this in due course; feel free to contact us with any questions.

# License

Corinthia is licensed under the Apache License version 2.0; see
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

[detailed status](https://github.com/uxproductivity/Corinthia/wiki/Incubator-proposal#current-status)

# Platforms and dependencies

Corinthia builds and runs on iOS, OS X, and Linux. Windows support is in the works.

To build DocFormats, you will need to have the following installed:

* [CMake](http://www.cmake.org)
* [libxml2](http://xmlsoft.org)
* [libiconv](https://www.gnu.org/software/libiconv/)
* [zlib](http://www.zlib.net)

# Build instructions

Corinthia currently build on Linux and OS X (mac).

For detailed instructions, please see

[build instructions](https://github.com/uxproductivity/Corinthia/wiki/Build-instructions)

# Further information

please see [mail list](https://github.com/uxproductivity/Corinthia/wiki/Talk-to-us) for contacts
