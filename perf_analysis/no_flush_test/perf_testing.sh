#!/bin/bash

datapath=/tmp/io_aggregation/perf_analysis/data/
#bounds rounded from results of "Towards Aggregation based I/O Optimization for Scaling Bioinformatics Applications"
writesize_lower_bound=10
writesize_avg=500
writesize_upper_bound=1000000
numwrites_lower_bound=200
numwrites_avg=136000
numwrites_upper_bound=10000000 
numfiles=1
bufsize=8000
num_data_pts=25


# testing flushing vs just destroying data and not flushing ==============
# to see if writing is even an issue at all. 
numwrites_upper_bound=10000000 
num_data_pts=50
size=1000

fname="no_flush.csv"
touch "${fname}"

echo -e "real,time,numwrites,size" >> "${fname}"
# lower to upper bound at evenly distrubuted intervals
for (( i=numwrites_lower_bound; i<numwrites_upper_bound; i+=$((numwrites_upper_bound/num_data_pts)) )); do
	time=$({ time LD_PRELOAD=./io_intercept.so ./test $i  1; }  2>&1 | grep real)
	echo -e "$time,$i,$size" >> "${fname}"
done
rm -rf ./junk/*



