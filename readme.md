* io_intercept.c 
	-contains the code to intercept open, read, write, fopen, fread, fwrite
	-logs filename, type of access, and number of bytes accessed to file_access_log.txt 

* compile with make, or:
	`gcc -shared -fPIC io_intercept.c -o io_intercept.so -ldl`

- this creates the interception library file, io_intercept.so

* run any program through this with:
	`LD_PRELOAD=$PWD/io_intercept.so ./program`

- this will capture all io from everything this program runs on

* todo: 
	 - catch a write
		  - capture the mode when opening a file (r/w/b/+)
	 - put in a data structure
	 - return status
	 - flush 1 write
	 - flush n writes
	 - catch a close()
	 - logging
	 - statistics (bytes read, etc)
	 - create a data structure for the io_aggregation_buf so multiple files can have their 
	    writes aggregated separately.
	 - catch a read() for a file that's currently being aggregated
	 - hdf5

