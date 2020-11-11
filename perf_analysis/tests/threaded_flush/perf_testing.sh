#!/bin/bash

datapath=./

#bounds rounded from results of "Towards Aggregation based I/O Optimization for Scaling Bioinformatics Applications"
writesize_lower_bound=10
writesize_avg=1000
writesize_upper_bound=100000

numwrites_lower_bound=1000
numwrites_avg=136000
numwrites_upper_bound=100000000

numfiles=1
bufsize=32000000
num_data_pts=25



fname="threaded_flush.csv"
make
echo -e "real,time,numwrites,size" >> "${datapath}${fname}"

for (( i=numwrites_lower_bound; i<numwrites_upper_bound; i+=$((numwrites_upper_bound/num_data_pts)) )); do
	time=$({ time ./test $i $writesize_avg $numfiles; }  2>&1 | grep real)
	echo -e "$time,$i,$writesize_avg" >> "${datapath}${fname}"
	rm ../../junk/*
done



