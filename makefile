
all: io_intercept.so test

io_intercept.so: io_intercept.c
	gcc -g -shared -fPIC  io_intercept.c -o io_intercept.so -ldl

test: test.c
	gcc -g test.c -o test

run: io_intercept.so test
	LD_PRELOAD=$(PWD)/io_intercept.so ./test

clean:
	rm test io_intercept.so
