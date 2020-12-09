#!/bin/bash


LD_PRELOAD=./filcio.so ./test 32 1 1

echo "\$ cat outfile.txt"
cat outfile.txt
echo ""
echo "\$ wc -l outfile.txt"
wc -l outfile.txt


