#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#define MAX_WRITES 10 // number of writes before flushing
#define MAX_BUF_SIZE 32000000 //total buffer size before flushing
#define MAX_WRITE_SIZE MAX_BUF_SIZE/MAX_WRITES //size in bytes allocated for each write


/* ===== STRUCTS ===== */

typedef struct file_buf{

	 int fd; //file descriptor
	 char* write_buf[MAX_WRITES]; // buffer containing up to 10 writes. each write can only be MAX_WRITE_SIZE long.
	 int num_writes;

} file_buf;

typedef struct master_aggregator{
	 file_buf* file_bufs;
} master_aggregator;

/* ===== uhhh ===== */

char* recover_filename(int fd);
void log_access(char* fname, char* type, size_t num_bytes);
int place_in_buf(file_buf fb, const void* buf, size_t size);
file_buf get_fb_by_fd(int fd);
void flush_write_buf();

/* ===== GLOBAL ===== */
master_aggregator master;



/* returns the file buffer associated with a file descriptor
 * Creates a new file_buf if not found
 */
file_buf get_fb_by_fd(int fd){
	 
	 for(int i=0; i<sizeof(master.file_bufs); i++){ // not sure if this loop will work
		  if(master.file_bufs[i].fd == fd){
				return master.file_bufs[i];
		  }
	 }

	 //file buffer for this fd doesn't exist
    file_buf new_fb;
    new_fb.fd = -1;
    new_fb.num_writes = 0;
    for(int i=0; i>MAX_WRITES; i++){
       new_fb.write_buf[i] = malloc(MAX_WRITE_SIZE);
    }

	 return new_fb;
}

/* places the data of a write() into the write buffer
 * returns 0 if successful, -1 if the write is too large
 *    writes too large should be written normally
 */
int place_in_buf(file_buf fb, const void* buf, size_t size){

	 if(size <= MAX_WRITE_SIZE){
		  fb.write_buf[fb.num_writes] = (char*) buf;
		  fb.num_writes++;

		  if(fb.num_writes == MAX_WRITES-1){
				flush_write_buf();
		  }
		  return 0;
	 }
	 return -1;
}



void flush_write_buf(file_buf fb){
	 FILE *f = fdopen(fb.fd, "wb+");
	 fwrite(fb.write_buf, sizeof(char), sizeof(fb.write_buf), f);
	 fclose(f);
	 fb.num_writes = 0;
}



// intercept main, allocate memory for the io buffer and return flow to normal main
int main(int argc, char* argv[]){
	 int (*orig_main)(int argc, char* argv[]) = dlsym(RTLD_NEXT, "main");

	 return orig_main(argc, argv);
}



/* ===== INTERCEPTION ===== */

int open(const char *filename, int flags, ...){
	int (*orig_open)(const char*, int) = dlsym(RTLD_NEXT,"open");
	return orig_open(filename,flags);
}

FILE* fopen(const char *filename, const char *mode){
	FILE* (*orig_fopen)(const char*, const char*) = dlsym(RTLD_NEXT, "fopen");
	return orig_fopen(filename, mode);
}

pid_t fork(){
	pid_t (*orig_fork)() = dlsym(RTLD_NEXT, "fork");
	return orig_fork();
}

ssize_t read(int fd, void *buf, size_t count){
	ssize_t (*orig_read)(int, void*, size_t) = dlsym(RTLD_NEXT, "read");
	return orig_read(fd, buf, count);
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream){
	size_t (*orig_fread)(void *, size_t, size_t, FILE*) = dlsym(RTLD_NEXT, "fread");
	return orig_fread(ptr, size, nmemb, stream);
}

ssize_t write(int fd, const void *buf, size_t count){
	 ssize_t (*orig_write)(int, const void*, size_t) = dlsym(RTLD_NEXT, "write");

	 file_buf fb = get_fb_by_fd(fd);
	 if(place_in_buf(fb, buf, count) == -1){ // write too big
		  return orig_write(fd, buf, count);
	 }
	 return 0; //what should the ret val be?
}

size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream){
	size_t (*orig_fwrite)(const void*, size_t, size_t, FILE*) = dlsym(RTLD_NEXT, "fwrite");

	 /*file_buf fb = get_fb_by_fd(fileno(stream));
	 if(place_in_buf(fb, ptr, size*nmemb) == -1){ // write too big
		  return orig_fwrite(ptr, size, nmemb, stream);	  	 
	 }*/
    printf("fwrite intercepted\n");
	 return 0; //what should the ret val be?
}


// gets the original filename from the file descriptor
// linux only (possibly most unix, untested)
// doesn't really work
char* recover_filename(int fd){
	char fd_path[256];
	sprintf(fd_path, "/proc/self/fd/%d", fd);
	char *filename = malloc(256);
	int n;
	if ((n = readlink(fd_path, filename, 255)) < 0)
	    return NULL;
	filename[n] = '\0';
	return filename;

}

void log_access(char* fname, char* type, size_t num_bytes){
	if(strcmp(fname,"file_access_log.txt") == 0) return; //prevent infinite loop
	FILE *f = fopen("file_access_log.txt", "ab+");
	if (f == NULL){
		printf("Couldn't log %s to file_access_log.txt\n", fname);
	}
	fprintf(f, "filename: %s, type: %s, bytes: %zu\n", fname, type, num_bytes);
	fclose(f);
}

