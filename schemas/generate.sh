#!/bin/bash

rm -rf output
mkdir output
./relaxng.js > output/collated.csv
./createimpl.js
#gperf -m 5 output/taglookup.gperf > output/taglookup1.c
#grep -v '^#line' output/taglookup1.c > output/taglookup.c
#rm -f output/taglookup1.c
