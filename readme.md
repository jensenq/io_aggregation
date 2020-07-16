* io_intercept.c 
	- intercepts i/o function calls
	-

* compile with make
* debug with gdb ./program_name
	* (gdb) set environment LD_PRELOAD ./io_intercept.so
	* (gdb) start

- this creates the interception library io_intercept.so

* run with make run

* todo: 
	* [x] catch a write
	* [x] intercept anything
	* [x] transparently return status
	* [x] create data structure for file buffer
	* [x] record write to memory
	* [x] flush 1 write to disk
	* [ ] flush n writes to disk
	* [ ] handle close()
	* [x] delete a file_buffer
	* [ ] make return values for interceptions MATCH the real versions so its transparent
	* [ ] catch a read() for a file that's currently being aggregated
	* [ ] hdf5
	* [ ] handle multiple processes

	* [ ] misc
		* [ ] logging
		* [ ] statistics (bytes read, etc)
		* [x] turn master's array of fb's into a linked list
		* [ ] be able to close a file after open() is called on it (so it doesnt stay open the whole time)
