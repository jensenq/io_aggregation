* io_intercept.c 
	-contains the code to intercept open, read, write, fopen, fread, fwrite
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
	* [ ] logging
	* [ ] statistics (bytes read, etc)
	* [ ] create a data structure for the io_aggregation_buf so multiple files can have their 
	*    writes aggregated separately.
	* [ ] catch a read() for a file that's currently being aggregated
	* [ ] hdf5
	* [ ] handle multiple processes

	* [ ] misc
		* [ ] make return values for interceptions MATCH the real versions so its transparent
		* [x] turn master's array of fb's into a linked list
		* [ ] delete a file_buffer
		* [ ] be able to close a file after open() is called on it (so it doesnt stay open the whole time)
