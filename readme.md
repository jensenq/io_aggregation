* About 
	- Many bioinformatics pipelines consist of black-box tools that each generate significant I/O. 
		When strung together by reading and writing files, each stage of the pipeline is bottlenecked by I/O, 
		without much ability to change that behavior. We propose a preloaded library to intercept I/O function 
		calls at a system level, aggregating their data in memory before writing to disk. This technique hopes 
		to scale I/O intensive bioinformatics pipelines without needing to access and modify the source code 
		of black-box tools that it may be comprised of.
	- FILCIO (File Interception Library for Collective I/O) intercepts function calls that write to disk, and 
		instead writes them to a double-buffer in memory. When this buffer is full, it then flushes to disk. 
		This changes the I/O characteristic of a program from large volume of small writes to a small volume of 
		large writes. 

* Getting Started:
	- compile the shared library file:
	```bash
		gcc -O3 -pthread -shared -fPIC  filcio.c -o filcio.so -ldl
	```
	- set the size of the buffer to use:
	```bash
		export AGG_BUFSIZE=32000000
	```
	- run your program with filcio preloaded:
	```bash
		LD_PRELOAD=./filcio.so ./my_prog
	```

* debug info:
	- preload filcio in gdb:
	```bash
		set environment LD_PRELOAD ./filcio.so
	```
	- crashing before main? 
	```bash
		ulimit -c unlimited
	```
	- profile function calls:
	```bash
		valgrind --trace-children=yes --tool=callgrind env LD_PRELOAD=./filcio.so ./test 
		callgrind_annotate callgrind.out.1178 --inclusive=yes --tree=both
	```

* Author:
	- Quentin Jensen

* special thanks to:
	- Dr Filip Jagodzinski 
	- Dr Tanzima Islam
