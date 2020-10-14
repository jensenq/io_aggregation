#!/bin/bash

datapath=/home/jensenq/research/perf_data/
#bounds rounded from results of "Towards Aggregation based I/O Optimization for Scaling Bioinformatics Applications"
writesize_lower_bound=10
writesize_avg=500
writesize_upper_bound=100000
numwrites_lower_bound=200
numwrites_avg=136000
numwrites_upper_bound=680000 
num_data_pts=25
numfiles=1
bufsize=8000

# ======== numwrites x interception x optimization =================================================
datapath=/home/jensenq/research/perf_data/writes_opt_test/

# [x] interception 
# [x] optimization
fname="numwrites_w_int_w_opt.csv"
gcc -O3 -shared -fPIC  io_intercept.c -o io_intercept.so -ldl
gcc -03 test.c -o test
echo -e "real,time,numwrites,size" >> "${datapath}${fname}"

# lower to upper bound at evenly distrubuted intervals
for (( i=numwrites_lower_bound; i<numwrites_upper_bound; i+=(numwrites_upper_bound/num_data_pts)); do
	time=$({ time LD_PRELOAD=./io_intercept.so ./test $i $writesize_avg; }  2>&1 | grep real)
	echo -e "$time,$i,$writesize_avg" >> "${datapath}${fname}"
done
rm -rf ./junk/*

#---------------------------------
# [ ] interception 
# [x] optimization
fname="numwrites_no_int_w_opt.csv"
gcc -03 test.c -o test
echo -e "real,time,numwrites,size" >> "${datapath}${fname}"

# lower to upper bound at evenly distrubuted intervals
for (( i=numwrites_lower_bound; i<numwrites_upper_bound; i+=(numwrites_upper_bound/num_data_pts)); do
	time=$({ time ./test $i $writesize_avg; }  2>&1 | grep real)
	echo -e "$time,$i,$writesize_avg" >> "${datapath}${fname}"
done
rm -rf ./junk/*

#---------------------------------
# [x] interception 
# [ ] optimization
fname="numwrites_w_int_no_opt.csv"
gcc -g -shared -fPIC  io_intercept.c -o io_intercept.so -ldl
gcc -g test.c -o test
echo -e "real,time,numwrites,size" >> "${datapath}${fname}"

# lower to upper bound at evenly distrubuted intervals
for (( i=numwrites_lower_bound; i<numwrites_upper_bound; i+=(numwrites_upper_bound/num_data_pts)); do
	time=$({ time LD_PRELOAD=./io_intercept.so ./test $i $writesize_avg; }  2>&1 | grep real)
	echo -e "$time,$i,$writesize_avg" >> "${datapath}${fname}"
done
rm -rf ./junk/*

#---------------------------------
# [ ] interception 
# [ ] optimization
fname="numwrites_no_int_no_opt.csv"
gcc -g test.c -o test
echo -e "real,time,numwrites,size" >> "${datapath}${fname}"

# lower to upper bound at evenly distrubuted intervals
for (( i=numwrites_lower_bound; i<numwrites_upper_bound; i+=(numwrites_upper_bound/num_data_pts)); do
	time=$({ time ./test $i $writesize_avg; }  2>&1 | grep real)
	echo -e "$time,$i,$writesize_avg" >> "${datapath}${fname}"
done
rm -rf ./junk/*

# ===================================================================================================









# === writesize ===
#echo -e "real\ttime    \tnumwrites\tsize" >> $fname
#for (( i=0; i<num_data_pts; i++)); do
#	time=$({ time LD_PRELOAD=./io_intercept.so ./test $numwrites $writesize; }  2>&1 | grep real)
#	echo -e "$time\t$numwrites\t$writesize" >> $fname
#	writesize=$(( writesize * 2 ))
#done

# === bufsize ===
#echo -e "real\ttime    \tnumwrites\twritesize\tbufsize" >> $fname
#for (( i=0; i<num_data_pts; i++)); do
#	export AGG_BUFSIZE=$bufsize
#	time=$({ time LD_PRELOAD=./io_intercept.so ./test $numwrites $writesize $numfiles; }  2>&1 | grep real)
#	echo -e "$time\t$numwrites\t$writesize\t$bufsize" >> $fname
#	bufsize=$(( bufsize * 2 ))
#done


# === numfiles ===
#echo -e "real\ttime    \tnumwrites\tsize\tnumfiles" >> $fname
#for (( i=0; i<num_data_pts; i++)); do
#	time=$({ time LD_PRELOAD=./io_intercept.so ./test $numwrites $writesize $numfiles; }  2>&1 | grep real)
#	echo -e "$time\t$numwrites\t$writesize\t$numfiles" >> $fname
#	numfiles=$(( numfiles * 2 ))
#done


rm -rf ./junk/*

	 
	

