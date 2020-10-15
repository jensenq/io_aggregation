* io_intercept.c 
	- intercepts i/o function calls
	-

* compile with make
* debug with gdb ./program_name
	* (gdb) set environment LD_PRELOAD ./io_intercept.so
	* (gdb) start
* crashing before main? use $ ulimit -c unlimited

- this creates the interception library io_intercept.so

* run with make run

* todo: 
	* [x] catch a write
	* [x] intercept anything
	* [x] transparently return status
	* [x] create data structure for file buffer
	* [x] record write to memory
	* [x] flush 1 write to disk
	* [x] flush n writes to disk
	* [x] handle a write when max capacity is reached
	* [x] handle close()
	* [x] delete a file_buffer
	* [x] make return values for interceptions MATCH the real versions so its transparent
	* [x] catch a read() for a file that's currently being aggregated
	* [x] test for high volume of small data
	* [x] test for med volume of med data
	* [x] test for low volume of large data
	* [x] test for non-string data
	* [ ] hdf5
	* [ ] handle multiple processes

	* [ ] misc
		* [ ] logging
		* [x] statistics (bytes read, etc)
		* [x] turn master's array of fb's into a linked list
