#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <stdbool.h> 
#include <sys/time.h>

#define MAX_BUF_SIZE 32000000 //total buffer size before flushing
#define DEBUG_LVL 1

/* ===== STRUCTS ===== */

typedef struct file_buf{
	const char* filename;
	const char* mode;
	int fd;
	size_t curr_size;         // how full the buffer is in bytes
	unsigned char* write_buf; // one contiguous chunk of memory
	struct file_buf* next;    // this is a linked list
} file_buf;


/* ===== GLOBAL ===== */
file_buf* global_fb_ptr = NULL;

int append_write(file_buf*, const void*, size_t);
file_buf* get_fb_by_fd(int);
void flush_buf(file_buf*);



/* returns a pointer to the file buffer associated with a file descriptor
 */
file_buf* get_fb_by_fd(int fd){
	 
	file_buf* tmp = global_fb_ptr;
	while(tmp != NULL){

		if(tmp->fd == fd){
			return tmp;
		}
		tmp = tmp->next;
	}

	return tmp;
}

/* places the data of a write() into the write buffer
 * if this new data will overflow the buffer, flush the buffer first.
 * returns 0 if successful, -1 if the write is too large for the buffer,
 * which should be written normally
 */
int append_write(file_buf* fb, const void* buf, size_t size){
	
	if(size <= MAX_BUF_SIZE){
		if(fb->curr_size + size > MAX_BUF_SIZE){
			if(DEBUG_LVL>=2){printf("not enough space remaining. data size: %li. curr_size: %li. max size: %i\n", size, fb->curr_size, MAX_BUF_SIZE);}
			flush_buf(fb);
		}
		
		memcpy(&fb->write_buf[fb->curr_size], buf, size);
		fb->curr_size += size;

		if(DEBUG_LVL>=3){printf("write recorded. file: %s, curr_size: %li\n", fb->filename, fb->curr_size);}
		return 0;
	}
	return -1;
}

/* creates a new file buffer, and adds it to the global list of file buffers
 *  note: inserted at the head of the list (files recently opened are more likely to be used soon) 
 */
void insert_fb(int fd, const char* filename, const char* mode){
	if(get_fb_by_fd(fd) != NULL){ //already exists
		if(DEBUG_LVL>=1){printf("Error: this file buffer is already in memory");}
	}
	else{
		file_buf* old_head = global_fb_ptr;
		file_buf* new_fb = (file_buf*) malloc(sizeof(file_buf));
		new_fb->filename = filename;
		new_fb->mode = mode;
		new_fb->fd = fd;
		new_fb->curr_size = 0;
		new_fb->write_buf = (unsigned char*)malloc(
			sizeof(unsigned char) * (MAX_BUF_SIZE+1)); 
		new_fb->next = old_head;
		global_fb_ptr = new_fb; 
	}
}

void flush_buf(file_buf* fb){

	struct timeval begin, end;
	size_t debug_bytes_written;
	gettimeofday(&begin, 0);
	debug_bytes_written = fb->curr_size;
	if(DEBUG_LVL>=3){printf("flushing %li bytes from %s buffer\n", fb->curr_size, fb->filename);}

	write(-1*fb->fd, fb->write_buf, fb->curr_size);
	memset(fb->write_buf, 0, fb->curr_size);
	fb->curr_size = 0;

	if(DEBUG_LVL>=1){
		gettimeofday(&end, NULL);
		long seconds = end.tv_sec - begin.tv_sec;
		long microseconds = end.tv_usec - begin.tv_usec;
		double elapsed = seconds + microseconds*1e-6;
		double mbytes_per_sec = (debug_bytes_written/elapsed)/1000000;
		printf("Time to flush %ld bytes: %.3f seconds. (%.3f MB/sec) \n", debug_bytes_written, elapsed, mbytes_per_sec);
	}
}


void delete_fb(file_buf* fb){
	free(fb->write_buf);
	free(fb);
}


/* ===== INTERCEPTION ===== */

FILE* fopen(const char *filename, const char *mode){
	FILE* (*orig_fopen)(const char*, const char*) = dlsym(RTLD_NEXT, "fopen");
	FILE* orig_retval = orig_fopen(filename, mode);
	insert_fb(fileno(orig_retval), filename, mode);
	return orig_retval;
}

size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream){
	size_t (*orig_fwrite)(const void*, size_t, size_t, FILE*) = dlsym(RTLD_NEXT, "fwrite");

	file_buf* fb = get_fb_by_fd(fileno(stream));
	
	int too_big_flag = append_write(fb, ptr, nmemb);
	if(too_big_flag == -1){ // write too big for buffer
		if(DEBUG_LVL>=3){printf("data too large. data size: %li. max size: %i\n", nmemb, MAX_BUF_SIZE);}
	   return orig_fwrite(ptr, size, nmemb, stream);	  	 
	}
	return nmemb;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream){
	size_t (*orig_fread)(void *, size_t, size_t, FILE*) = dlsym(RTLD_NEXT, "fread");
	int fd = fileno(stream);
	file_buf* fb = get_fb_by_fd(fd);
	if(DEBUG_LVL>=3){printf("reading %s\n", fb->filename);}
	flush_buf(fb);
	return orig_fread(ptr, size, nmemb, stream);
}

int fclose(FILE* stream){
	int (*orig_fclose)(FILE*) = dlsym(RTLD_NEXT, "fclose");
	int fd = fileno(stream);
	file_buf* fb = get_fb_by_fd(fd);
	if(DEBUG_LVL>=3){printf("closing %s\n", fb->filename);}
	flush_buf(fb);
	if(DEBUG_LVL>=3){printf("%s closed.\n", fb->filename);}
	delete_fb(fb);
	return orig_fclose(stream); 
}

int open(const char *filename, int flags, ...){
	int (*orig_open)(const char*, int) = dlsym(RTLD_NEXT,"open");
	int orig_retval = orig_open(filename, flags);
	insert_fb(orig_retval, filename, "");
	return orig_retval;
}

ssize_t read(int fd, void *buf, size_t count){
	ssize_t (*orig_read)(int, void*, size_t) = dlsym(RTLD_NEXT, "read");
	file_buf* fb = get_fb_by_fd(fd);
	if(DEBUG_LVL>=3){printf("reading %s\n", fb->filename);}
	flush_buf(fb);
	return orig_read(fd, buf, count);
}

ssize_t write(int fd, const void *buf, size_t count){
	ssize_t (*orig_write)(int, const void*, size_t) = dlsym(RTLD_NEXT, "write");

	file_buf* fb = get_fb_by_fd(fd);

	//negative fd signals this is a normal write.
	if(fd<0){
	   return orig_write(-fd, buf, count);
	}
	else{
		int too_big_flag = append_write(fb, buf, count);
		if(too_big_flag == -1){ // write too big for buffer
			if(DEBUG_LVL>=3){printf("data too large, writing to disk... data size: %li. max size: %i\n", count, MAX_BUF_SIZE);}
			return orig_write(fd, buf, count);
		}
	}

	return count;
}

int close(int fd){
	int (*orig_close)(int) = dlsym(RTLD_NEXT, "close");
	file_buf* fb = get_fb_by_fd(fd);
	flush_buf(fb);
	delete_fb(fb);
	return orig_close(fd); 
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


//debug functions
int main(){
	file_buf* fb = (file_buf*) malloc(sizeof(file_buf));
	fb->filename = "";
	fb->mode = "";
	fb->fd = 0;
	fb->curr_size = 0;
	fb->write_buf = (unsigned char*)malloc(
		sizeof(unsigned char) * (MAX_BUF_SIZE+1)); 
	fb->next = NULL;

	flush_buf(fb);
	delete_fb(fb);

}

