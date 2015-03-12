from distutils.core import setup, Extension

# define the extension module
dfutil_module = Extension('dfutil', 
    sources=['src/dfutil.c', '../../consumers/dfutil/src/Commands.c'],
    libraries = ['DocFormats', 'xml2'],
    extra_compile_args = ['-std=c99'],
    library_dirs = ['../../build/lib'],
    include_dirs = [
        '../../DocFormats/api/headers', 
        '../../DocFormats/core/src/common',
        '../../DocFormats/core/src/css', 
        '../../DocFormats/core/src/html', 
        '../../DocFormats/core/src/lib', 
        '../../DocFormats/core/src/names', 
        '../../DocFormats/core/src/xml', 
        '../../DocFormats/core/tests/common', 
        '../../DocFormats/core/tests/html',
        '../../DocFormats/filters/latex/src',
        '../../DocFormats/filters/ooxml/src/common', 
        '../../DocFormats/filters/ooxml/src/word', 
        '../../DocFormats/filters/ooxml/tests/word',
        '../../DocFormats/headers',
    ]
)

dfconvert_module = Extension('dfconvert', 
    sources=['src/dfconvert.c'],
    libraries = ['DocFormats', 'xml2', 'SDL2', 'SDL_image'],
    extra_compile_args = ['-std=c99'],
    library_dirs = ['../../build/lib'],
    include_dirs = [
        '../../DocFormats/api/headers', 
        '../../DocFormats/core/src/common',
        '../../DocFormats/core/src/css', 
        '../../DocFormats/core/src/html', 
        '../../DocFormats/core/src/lib', 
        '../../DocFormats/core/src/names', 
        '../../DocFormats/core/src/xml', 
        '../../DocFormats/core/tests/common', 
        '../../DocFormats/core/tests/html',
        '../../DocFormats/filters/latex/src',
        '../../DocFormats/filters/ooxml/src/common', 
        '../../DocFormats/filters/ooxml/src/word', 
        '../../DocFormats/filters/ooxml/tests/word',
        '../../DocFormats/headers',
    ]
)

# run the setup
setup(ext_modules=[dfutil_module, dfconvert_module])  
