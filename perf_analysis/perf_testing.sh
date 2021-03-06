#!/bin/bash

datapath=/tmp/io_aggregation/perf_analysis/data/

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



fname="stress_wo_int.csv"
make
echo -e "real,time,numwrites,size" >> "${datapath}${fname}"

for (( i=numwrites_lower_bound; i<numwrites_upper_bound; i+=$((numwrites_upper_bound/num_data_pts)) )); do
	for (( j=writesize_lower_bound; j<writesize_upper_bound; j+=$((writesize_upper_bound/num_data_pts)) )); do
		time=$({ time ./test $i $j 1; }  2>&1 | grep real)
		echo -e "$time,$i,$j" >> "${datapath}${fname}"
		rm -rf ./junk/*
	done
done



#cd ..
#
#fname="stress_w_int.csv"
#make
#echo -e "real,time,numwrites,size" >> "${datapath}${fname}"
#
#for (( i=numwrites_lower_bound; i<numwrites_upper_bound; i+=$((numwrites_upper_bound/num_data_pts)) )); do
#	for (( j=writesize_lower_bound; j<writesize_upper_bound; j+=$((writesize_upper_bound/num_data_pts)) )); do
#		time=$({ time LD_PRELOAD=./io_intercept.so ./test $i $j 1; }  2>&1 | grep real)
#		echo -e "$time,$i,$j" >> "${datapath}${fname}"
#		rm -rf ./junk/*
#	done
#done




## ===== testing flushing vs just destroying data and not flushing ========================================
## to see if writing is even an issue at all. 
#numwrites_upper_bound=10000000 
#num_data_pts=50
#
#cd ..
#fname="with_flush.csv"
#touch "${datapath}${fname}"
#make
#echo -e "real,time,numwrites,size" >> "${datapath}${fname}"
#size=$(($disk_cache+1))
## lower to upper bound at evenly distrubuted intervals
#for (( i=numwrites_lower_bound; i<numwrites_upper_bound; i+=$((numwrites_upper_bound/num_data_pts)) )); do
#	time=$({ time LD_PRELOAD=./io_intercept.so ./test $i size 1; }  2>&1 | grep real)
#	echo -e "$time,$i,$size" >> "${datapath}${fname}"
#done
#rm -rf ./junk/*
#
#
#cd ./perf_analysis/no_flush_test
#fname="no_flush.csv"
#touch "${datapath}${fname}"
#
#echo -e "real,time,numwrites,size" >> "${datapath}${fname}"
#size=$(($disk_cache+1))
## lower to upper bound at evenly distrubuted intervals
#for (( i=numwrites_lower_bound; i<numwrites_upper_bound; i+=$((numwrites_upper_bound/num_data_pts)) )); do
#	time=$({ time LD_PRELOAD=./io_intercept.so ./test $i size 1; }  2>&1 | grep real)
#	echo -e "$time,$i,$size" >> "${datapath}${fname}"
#done
#rm -rf ./junk/*




# ======== disk cache testing ==============================================================================
#disk_cache=8192000
#datapath=~/research/io_aggregation/perf_analysis/data/disk_cache_test/
#
#
#cd ..
#
#fname="over_cache.csv"
#touch "${datapath}${fname}"
#gcc -shared -fPIC  io_intercept.c -o io_intercept.so -ldl
#gcc test.c -o test
#echo -e "real,time,numwrites,size" >> "${datapath}${fname}"
#size=$(($disk_cache+1))
## lower to upper bound at evenly distrubuted intervals
#for (( i=numwrites_lower_bound; i<numwrites_upper_bound; i+=$((numwrites_upper_bound/num_data_pts)) )); do
#	time=$({ time LD_PRELOAD=./io_intercept.so ./test $i size 1; }  2>&1 | grep real)
#	echo -e "$time,$i,$size" >> "${datapath}${fname}"
#done
#rm -rf ./junk/*
#
#fname="under_cache.csv"
#touch "${datapath}${fname}"
#gcc -shared -fPIC  io_intercept.c -o io_intercept.so -ldl
#gcc test.c -o test
#echo -e "real,time,numwrites,size" >> "${datapath}${fname}"
#size=$(($disk_cache-1))
#
## lower to upper bound at evenly distrubuted intervals
#for (( i=numwrites_lower_bound; i<numwrites_upper_bound; i+=$((numwrites_upper_bound/num_data_pts))  )); do
#	time=$({ time LD_PRELOAD=./io_intercept.so ./test $i size 1; }  2>&1 | grep real)
#	echo -e "$time,$i,$size" >> "${datapath}${fname}"
#done
#rm -rf ./junk/*



##
## ======== numwrites x interception x optimization =================================================
#cd ..
#datapath=/home/jensenq/research/perf_data/writes_opt_test/
#
## [x] interception 
## [x] optimization
#fname="numwrites_w_int_w_opt.csv"
#gcc -O3 -shared -fPIC  io_intercept.c -o io_intercept.so -ldl
#gcc -03 test.c -o test
#echo -e "real,time,numwrites,size" >> "${datapath}${fname}"
#
## lower to upper bound at evenly distrubuted intervals
#for (( i=numwrites_lower_bound; i<numwrites_upper_bound; i+=(numwrites_upper_bound/num_data_pts)); do
#	time=$({ time LD_PRELOAD=./io_intercept.so ./test $i $writesize_avg; }  2>&1 | grep real)
#	echo -e "$time,$i,$writesize_avg" >> "${datapath}${fname}"
#done
#rm -rf ./junk/*
#
##---------------------------------
## [ ] interception 
## [x] optimization
#fname="numwrites_no_int_w_opt.csv"
#gcc -03 test.c -o test
#echo -e "real,time,numwrites,size" >> "${datapath}${fname}"
#
## lower to upper bound at evenly distrubuted intervals
#for (( i=numwrites_lower_bound; i<numwrites_upper_bound; i+=(numwrites_upper_bound/num_data_pts)); do
#	time=$({ time ./test $i $writesize_avg; }  2>&1 | grep real)
#	echo -e "$time,$i,$writesize_avg" >> "${datapath}${fname}"
#done
#rm -rf ./junk/*
#
##---------------------------------
## [x] interception 
## [ ] optimization
#fname="numwrites_w_int_no_opt.csv"
#gcc -g -shared -fPIC  io_intercept.c -o io_intercept.so -ldl
#gcc -g test.c -o test
#echo -e "real,time,numwrites,size" >> "${datapath}${fname}"
#
## lower to upper bound at evenly distrubuted intervals
#for (( i=numwrites_lower_bound; i<numwrites_upper_bound; i+=(numwrites_upper_bound/num_data_pts)); do
#	time=$({ time LD_PRELOAD=./io_intercept.so ./test $i $writesize_avg; }  2>&1 | grep real)
#	echo -e "$time,$i,$writesize_avg" >> "${datapath}${fname}"
#done
#rm -rf ./junk/*
#
##---------------------------------
## [ ] interception 
## [ ] optimization
#fname="numwrites_no_int_no_opt.csv"
#gcc -g test.c -o test
#echo -e "real,time,numwrites,size" >> "${datapath}${fname}"
#
## lower to upper bound at evenly distrubuted intervals
#for (( i=numwrites_lower_bound; i<numwrites_upper_bound; i+=(numwrites_upper_bound/num_data_pts)); do
#	time=$({ time ./test $i $writesize_avg; }  2>&1 | grep real)
#	echo -e "$time,$i,$writesize_avg" >> "${datapath}${fname}"
#done
#rm -rf ./junk/*
#
## ===================================================================================================









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

	 
	

