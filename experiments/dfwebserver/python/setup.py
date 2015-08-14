#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

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
