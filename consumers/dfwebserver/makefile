
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
