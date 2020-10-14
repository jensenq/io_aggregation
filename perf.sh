#!/bin/bash

fname=/home/jensenq/research/time_vs_bufsize.log
num_data_pts=25
numwrites=1000000
writesize=1000
numfiles=1
bufsize=8000

#numwrites
#echo -e "real\ttime    \tnumwrites\tsize" >> time_vs_numwrites.log
#for (( i=0; i<num_data_pts; i++)); do
#	time=$({ time LD_PRELOAD=./io_intercept.so ./test $numwrites $size; }  2>&1 | grep real)
#	echo -e "$time\t$numwrites\t$size" >> time_vs_numwrites.log
#	numwrites=$(( numwrites * 2 ))
#done

#writesize
#echo -e "real\ttime    \tnumwrites\tsize" >> $fname
#for (( i=0; i<num_data_pts; i++)); do
#	time=$({ time LD_PRELOAD=./io_intercept.so ./test $numwrites $writesize; }  2>&1 | grep real)
#	echo -e "$time\t$numwrites\t$writesize" >> $fname
#	writesize=$(( writesize * 2 ))
#done

#bufsize
#echo -e "real\ttime    \tnumwrites\twritesize\tbufsize" >> $fname
for (( i=0; i<num_data_pts; i++)); do
	export AGG_BUFSIZE=$bufsize
	time=$({ time LD_PRELOAD=./io_intercept.so ./test $numwrites $writesize $numfiles; }  2>&1 | grep real)
	echo -e "$time\t$numwrites\t$writesize\t$bufsize" >> $fname
	bufsize=$(( bufsize * 2 ))
done


#numfiles
#echo -e "real\ttime    \tnumwrites\tsize\tnumfiles" >> $fname
#for (( i=0; i<num_data_pts; i++)); do
#	time=$({ time LD_PRELOAD=./io_intercept.so ./test $numwrites $writesize $numfiles; }  2>&1 | grep real)
#	echo -e "$time\t$numwrites\t$writesize\t$numfiles" >> $fname
#	numfiles=$(( numfiles * 2 ))
#done




#{ time LD_PRELOAD=./io_intercept.so ./test 1 1; } 2>&1 | grep real >> time_vs_numwrites.log
	 
	

