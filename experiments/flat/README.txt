Flat (Formal LAanguage Transformation) is an experimental programming language
for parsing, transforming, and pretty-printing any data format that can be
expressed as a Parsing Expression Grammar (PEG) [1].

Its purpose is to provide an easy way of dealing with file formats like
Markdown, LaTeX, and org-mode which use a custom grammar, rather than a
standardized meta-format like XML or JSON.

Initially, the intention is to simply provide a parser interpreter (similar to a
parser generator of the bison/flex variety) such that we can get an abstract
syntax tree from a file, from which further processing can be done. The plan is
to later extend it to include a domain-specific language for expressing
transformations between different grammars, similar to Stratego/XT. [2]

To understand PEGs, read the following papers:

"Parsing Expression Grammars: A Recognition-Based Syntactic Foundation". Bryan
Ford, POPL, January 2004. http://bford.info/pub/lang/peg.pdf

"Packrat Parsing: Simple, Powerful, Lazy, Linear Time". Bryan Ford. ICFP,
October 2002. http://bford.info/pub/lang/packrat-icfp02.pdf

[1] http://bford.info/packrat

[2] http://strategoxt.org
