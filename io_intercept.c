#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#define MAX_BUF_SIZE 32000000 //total buffer size before flushing


/* ===== STRUCTS ===== */

typedef struct file_buf{
	int fd;
	size_t curr_size;         // how full the buffer is in bytes
	unsigned char* write_buf; // one contiguous chunk of memory
	struct file_buf* next;    // this is also just a node in a linked list of fb's
} file_buf;

typedef struct master_aggregator{
	file_buf* file_bufs;
} master_aggregator;

/* ===== */
int append_write(file_buf* fb, const void* buf, size_t size);
file_buf* get_fb_by_fd(int fd);
void flush_buf();

/* ===== GLOBAL ===== */
master_aggregator master;



/* returns a pointer to the file buffer associated with a file descriptor
 */
file_buf* get_fb_by_fd(int fd){
	 
	file_buf* fbp = master.file_bufs;
	while(fbp != NULL){

		if(fbp->fd = fd){
			return fbp;
		}
		fbp = fbp->next;
	}

	return NULL;
}

/* places the data of a write() into the write buffer
 * returns 0 if successful, -1 if the write is too large 
 * (writes too large should be written normally)
 */
int append_write(file_buf* fb, const void* buf, size_t size){

	if(size <= MAX_BUF_SIZE){
		if(fb->curr_size + size > MAX_BUF_SIZE){
			flush_buf();
		}
		
		//copy data, mark location in memory
		return 0;
	}
	return -1;
}

/* creates a new file buffer, and adds it to the global list of file buffers
 *  note: inserted at the head (files recently opened are more likely to be opened soon) 
 */
void insert_fb(int fd){
	if(get_fb_by_fd(fd) != NULL){ //already exists
		printf("Warning to dev: this file buffer is already in memory");
	}
	else{
		file_buf new_fb = {fd, 0, NULL, master.file_bufs}; 
		master.file_bufs = &new_fb;
	}
}


void flush_buf(file_buf* fb){
	 FILE *f = fdopen(fb->fd, "wb+");
	 fwrite(fb->write_buf, sizeof(unsigned char), sizeof(fb->write_buf), f);
	 fclose(f);
	 fb->curr_size = 0;
}

/* --- i'm not sure if main can be intercepted or not...

// intercept main, allocate memory for the io buffer and return flow to normal main
int main(){
	int (*orig_main)() = dlsym(RTLD_NEXT, "main");
	
	master = (master_aggregator) {0, NULL};
	printf("main without args intercepted\n");

	return orig_main();
}
int main(int argc, char* argv[]){
	int (*orig_main)(int argc, char* argv[]) = dlsym(RTLD_NEXT, "main");
	
	master = (master_aggregator) {0, NULL};
	printf("main with args intercepted\n");

	return orig_main(argc, argv);
}
*/


/* ===== INTERCEPTION ===== */

int open(const char *filename, int flags, ...){
	int (*orig_open)(const char*, int) = dlsym(RTLD_NEXT,"open");
	return orig_open(filename,flags);
}

FILE* fopen(const char *filename, const char *mode){
	FILE* (*orig_fopen)(const char*, const char*) = dlsym(RTLD_NEXT, "fopen");
	//printf("fopen intercepted\n");
	FILE* orig_retval = orig_fopen(filename, mode);
	insert_fb(fileno(orig_retval));
	return orig_retval;
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

	 file_buf* fb = get_fb_by_fd(fd);
	 if(append_write(fb, buf, count) == -1){ // write too big
		  return orig_write(fd, buf, count);
	 }
	 return 0; //what should the ret val be?
}

size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream){
	int x = 3;
	//size_t (*orig_fwrite)(const void*, size_t, size_t, FILE*) = dlsym(RTLD_NEXT, "fwrite");
	printf("fwrite intercepted\n");
	int fd = fileno(stream);
	file_buf* fb = get_fb_by_fd(fd);
	
	if(append_write(fb, ptr, size*nmemb) == -1){ // write too big
	   return orig_fwrite(ptr, size, nmemb, stream);	  	 
	}
	return 0;//orig_fwrite(ptr, size, nmemb, stream);
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

