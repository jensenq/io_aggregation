
all: io_intercept.so test 

io_intercept.so: io_intercept.c
	gcc -shared -fPIC  io_intercept.c -o io_intercept.so -ldl

test: test.c
	gcc test.c -o test

run: io_intercept.so test
	LD_PRELOAD=$(PWD)/io_intercept.so ./test
