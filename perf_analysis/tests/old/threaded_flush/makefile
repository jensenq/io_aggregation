
all: io_intercept.so test 

io_intercept.so: io_intercept.c
	gcc -O3 -g -pthread -shared -fPIC  io_intercept.c -o io_intercept.so -ldl

debug: io_intercept.c
	gcc -g test.c -o test
	gcc -g -pthread -shared -fPIC  io_intercept.c -o io_intercept.so -ldl

test: test.c
	gcc -O3 -g test.c -o test

run: io_intercept.so test
	export AGG_BUFSIZE=32000000
	LD_PRELOAD=$(PWD)/io_intercept.so ./test

clean:
	rm test io_intercept.so perf_analysis/junk/*



