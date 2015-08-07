# About Apache Corinthia (incubating)

Corinthia is a set of libraries and tools for dealing with different file
formats for productivity applications, with an initial focus on word processing.
The goal of the project is to provide components which developers can easily
integrate into their own applications and scripts for converting and
manipulating data in a wide range of formats via a consistent interface.

This is the first public release of Corinthia, and consists of a single core
library called DocFormats. The library provides two-way conversion between OOXML
word processing documents (aka Microsoft Word .docx) and HTML. Basic support for
converting HTML to LaTeX is also provided, and support for ODF is in its early
stages. The Microsoft Word support has previously been used in commercial
applications and is fairly mature.

The Corinthia project is part of the Apache Software Foundation
[incubator](http://incubator.apache.org/incubation/Process_Description.html),
which it entered on December 8, 2014. The [accepted
proposal](http://wiki.apache.org/incubator/CorinthiaProposal) and [incubation
status](http://incubator.apache.org/projects/corinthia.html) provide incubation
background and progress information.

The communication hub of the project is the development mailing list,

    dev @ corinthia.incubator.apache.org

To receive list postings and interact on the list, simply send a message to

   dev-subscribe @ corinthia.incubator.apache.org

from the email address to receive list messages at.  The reply from
the list robot to that address provides confirmation instructions and
information on managing the subscription.

Further links:

- [Corinthia incubator web site](http://corinthia.incubator.apache.org/)
- [Project wiki](http://incubator.apache.org/projects/corinthia.html), and a
- [JIRA issue tracker](https://issues.apache.org/jira/browse/COR).

These sites and the documentation for this project are at a preliminary stage.
Content will be moved to Apache and improved as incubation moves along.

There is also a [Facebook page](https://www.facebook.com/CorinthiaProject) and a
Twitter account, [@ApacheCorinthia](https://twitter.com/ApacheCorinthia).

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

* `DocFormats` - file format conversion library
* `dfconvert` - driver program for performing conversions
* `dftest` - test harness

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

Corinthia builds and runs on iOS, OS X, Linux and Windows.

To build DocFormats, you will need to have the following installed:

* [CMake](http://www.cmake.org)
* [libxml2](http://xmlsoft.org)
* [libiconv](https://www.gnu.org/software/libiconv/)
* [zlib](http://www.zlib.net)

# Build instructions

Corinthia currently builds on Linux, OS X and Windows. See the [build  instructions](https://cwiki.apache.org/confluence/display/Corinthia/Build+instructions).

# Contributing

Contributors are welcome and prized.  Details on how to participate on the
project will be posted soon.

Meanwhile, the easiest way to contribute is by subscribing to the development
list and asking your questions and offering suggestions there.


