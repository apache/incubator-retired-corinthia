# About Apache Corinthia (incubating)

Corinthia is a library for converting between different word-processing file
formats. Initially, it supports .docx (part of the OOXML specification), HTML,
and LaTeX (export-only). The Corinthia project also provides convenience
executables.  The library has shipped as part of
[UX Write](http://www.uxproductivity.com/) since February 2013.

On December 8, 2014, Corinthia entered the Apache Software
Foundation
[incubator](http://incubator.apache.org/incubation/Process_Description.html).
The
[accepted proposal](http://wiki.apache.org/incubator/CorinthiaProposal) and
[incubation status](http://incubator.apache.org/projects/corinthia.html)
provide incubation background and progress information.

The communication hub of the project is the development mailing list,

    dev @ corinthia.incubator.apache.org

To receive list postings and interact on the list, simply send a message to

   dev-subscribe @ corinthia.incubator.apache.org

from the email address to receive list messages at.  The reply from
the list robot to that address provides confirmation instructions and
information on managing the subscription.

There are a [Corinthia incubator web
site](http://corinthia.incubator.apache.org/), a
[project wiki](http://incubator.apache.org/projects/corinthia.html), and a
[JIRA issue tracker](https://issues.apache.org/jira/browse/COR).


The sites and documentation for this project are at a preliminary
stage. Content will be moved to Apache and improved as incubation moves
along.

Meanwhile, there is a [Facebook page](https://www.facebook.com/CorinthiaProject) and a Twitter account, [@ApacheCorinthia](https://twitter.com/ApacheCorinthia).

# License

Corinthia is licensed under the Apache License version 2.0; see
LICENSE.txt for details.

# What the library can do

1. Create new HTML files from a .docx source
2. Create new .docx files from a HTML source
3. Update existing .docx files based on a modified HTML file produced in (1)
4. Convert .docx or HTML files to LaTeX
5. Provide access to document structure, in terms of a DOM-like API for
   manipulating XML trees, and an object model for working with CSS
   stylesheets

# Components

There are three major components, in their respective directories:

* `DocFormats` - the library itself
* `dfutil` - a driver program used for running [...]
* automated tests (located in the tests directory)

Run dfutil without any command-line arguments to see a list of operations.
Here is an example of converting a .docx file to HTML, modifying it, and then
updating the original .docx. Note that it is important, due to how internal
mapping works, that the .docx file being written is the same file as the
original; using a new file won't work.

    dfutil filename.docx filename.html
    vi filename.html # Make some changes
    dfutil filename.html filename.docx

If you examine the convertFile function in `dfutil/Commands.c`, you will see
the main entry points to perform these conversions, which you can call from
your own program.

# Platforms and dependencies

Corinthia builds and runs on iOS, OS X, and Linux. Windows support is in the
works.

To build DocFormats, you will need to have the following installed:

* [CMake](http://www.cmake.org)
* [libxml2](http://xmlsoft.org)
* [libiconv](https://www.gnu.org/software/libiconv/)
* [zlib](http://www.zlib.net)

# Build instructions

Corinthia currently builds on Linux and OS X (mac). See the [build  instructions](https://github.com/uxproductivity/Corinthia/wiki/Build-instructions).

# Contributing

Contributors are welcome and prized.  Details on how to participate on the
project will be posted soon.

Meanwhile, the easiest way to contribute is by subscribing to the development
list and asking your questions and offering suggestions there.


