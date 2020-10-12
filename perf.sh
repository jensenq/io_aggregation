#!/bin/bash

num_data_pts=10
numwrites=10
size=1000

#numwrites
echo -e "real\ttime\tnumwrites" >> time_vs_numwrites.log
for (( i=0; i<num_data_pts; i++)); do
	#{ time LD_PRELOAD=./io_intercept.so ./test 1 1; } 2>&1 | grep real >> time_vs_numwrites.log
	time=$({ time LD_PRELOAD=./io_intercept.so ./test $numwrites $size; }  2>&1 | grep real)
	echo -e "$time\t$numwrites \n" >> time_vs_numwrites.log
	numwrites=$(( numwrites * 10 ))
done
	 
	

