#!/bin/bash

datapath=./

#bounds rounded from results of "Towards Aggregation based I/O Optimization for Scaling Bioinformatics Applications"
writesize_lower_bound=10
writesize_avg=500
writesize_upper_bound=100000

numwrites_lower_bound=1000000
numwrites_avg=136000
numwrites_upper_bound=7400000

numfiles=1
bufsize=8000
num_data_pts=25



fname="threaded_stress.csv"
make
echo -e "real,time,numwrites,size" >> "${datapath}${fname}"

for (( i=numwrites_lower_bound; i<numwrites_upper_bound; i+=$((numwrites_upper_bound/num_data_pts)) )); do
	for (( j=writesize_lower_bound; j<writesize_upper_bound; j+=$((writesize_upper_bound/num_data_pts)) )); do
		time=$({ time ./test $i $j 1; }  2>&1 | grep real)
		echo -e "$time,$i,$j" >> "${datapath}${fname}"
		rm -rf ../../junk/*
	done
done



