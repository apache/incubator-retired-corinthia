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


#build
#  make

#install
#  sudo python setup.py install

#unistall 
#  sudo python setup.py install --record files.txt
#  cat files.txt | sudo xargs rm -rf      

#develoment build
#  make clean dev
#  python test.py

LDFLAGS ?= ""
CFLAGS ?= ""

.PHONY: default build_ext clean

default: build_ext

dev:
	CFLAGS=$(CFLAGS) LDFLAGS=$(LDFLAGS) python setup.py build_ext --inplace --debug

build_ext:
	CFLAGS=$(CFLAGS) LDFLAGS=$(LDFLAGS) python setup.py build_ext --debug

clean:
	- rm -rf build
	- rm -rf consumers
	- rm -f output*.html
	- rm -f output*.docx
	- rm -f dummy*.docx
	- find ./ -name '*.so' -delete
