* filcio.c 
	- intercepts writes to disk and instead writes them to a buffer in memory. When this buffer is full, then it flushes to disk.
	- this is a library to be preloaded as follows:

* Usage:
	-  gcc -O3 -pthread -shared -fPIC  filcio.c -o filcio.so -ldl
	- export AGG_BUFSIZE=< bytes to allocate for each file buffer, default=32000000 >
	- LD_PRELOAD=./filcio.so 
	- ./test
	- or simply make run

* debug info:
	- (gdb) set environment LD_PRELOAD ./filcio.so
	- crashing before main? use $ ulimit -c unlimited

* Author:
	- Quentin Jensen

* special thanks to:
	- Dr Filip Jagodzinski 
	- Dr Tanzima Islam
