* io_intercept.c 
	- intercepts writes to disk and instead writes them to a buffer in memory. When this buffer is full, then it flushes to disk.
	- this is a library to be preloaded as follows:

* Usage:
	-  gcc -O3 -pthread -shared -fPIC  io_intercept.c -o io_intercept.so -ldl
	- export AGG_BUFSIZE=32000000
	- LD_PRELOAD=./io_intercept.so 
	- ./test
	- 
	- or simply make run


* debug info:
	- (gdb) set environment LD_PRELOAD ./io_intercept.so
	- crashing before main? use $ ulimit -c unlimited

* Author:
	- Quentin Jensen

* special thanks to:
	- Dr Filip Jagodzinski 
	- Dr Tanzima Islam
