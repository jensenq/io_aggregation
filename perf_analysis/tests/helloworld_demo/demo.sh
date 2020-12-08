#!/bin/bash


LD_PRELOAD=./filcio.so ./test 32 1 1

echo ""
echo "wc -l outfile.txt"
wc -l outfile.txt


