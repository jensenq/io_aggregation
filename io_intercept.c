#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <stdbool.h> 

#define MAX_BUF_SIZE 60 //total buffer size before flushing
#define DEBUG_ON true

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
static const char NO_INTERCEPT_FLAG[10] = "NOINTRCPT";

int append_write(file_buf*, const void*, size_t);
file_buf* get_fb_by_fd(int);
void flush_buf(file_buf*);
bool treat_as_normal(const void*, file_buf*);



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
			if(DEBUG_ON){printf("not enough space remaining. data size: %li. curr_size: %li. max size: %i\n", size, fb->curr_size, MAX_BUF_SIZE);}
			flush_buf(fb);
		}
		
		memcpy(&fb->write_buf[fb->curr_size], buf, size);
		fb->curr_size += size;
		return 0;
	}
	return -1;
}

/* creates a new file buffer, and adds it to the global list of file buffers
 *  note: inserted at the head (files recently opened are more likely to be opened soon) 
 */
void insert_fb(int fd, const char* filename, const char* mode){
	if(get_fb_by_fd(fd) != NULL){ //already exists
		if(DEBUG_ON){printf("this file buffer is already in memory");}
	}
	else{
		file_buf* old_head = global_fb_ptr;
		file_buf* new_fb = (file_buf*) malloc(sizeof(file_buf));
		new_fb->filename = filename;
		new_fb->mode = mode;
		new_fb->fd = fd;
		new_fb->curr_size = 0;
		new_fb->write_buf = (unsigned char*)malloc(
			sizeof(unsigned char) * (MAX_BUF_SIZE + sizeof(NO_INTERCEPT_FLAG)+1)); 
		new_fb->next = old_head;
		global_fb_ptr = new_fb; 
	}
}

void flush_buf(file_buf* fb){

	if(DEBUG_ON){printf("flushing %li bytes from %s buffer\n", fb->curr_size, fb->filename);}
	//append the flag so write() is treated as normal
	strcat(&(fb->write_buf[fb->curr_size]), NO_INTERCEPT_FLAG);
	write(fb->fd, fb->write_buf, fb->curr_size+sizeof(NO_INTERCEPT_FLAG));

	//do we need to zero out the write buf?
	memset(fb->write_buf, 0, fb->curr_size);
	fb->curr_size = 0;

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

	if(treat_as_normal(ptr, fb)){
		return orig_fwrite(ptr, size, nmemb, stream);
	}
	else{	
		int tmp = append_write(fb, ptr, nmemb);
		if(tmp == -1){ // write too big for buffer
			if(DEBUG_ON){printf("data too large. data: \"%s\". max size: %i\n", (const char*)ptr, MAX_BUF_SIZE);}
		   return orig_fwrite(ptr, size, nmemb, stream);	  	 
		}
	}
	return nmemb;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream){
	size_t (*orig_fread)(void *, size_t, size_t, FILE*) = dlsym(RTLD_NEXT, "fread");
	int fd = fileno(stream);
	file_buf* fb = get_fb_by_fd(fd);
	flush_buf(fb);
	return orig_fread(ptr, size, nmemb, stream);
}

int fclose(FILE* stream){
	int (*orig_fclose)(FILE*) = dlsym(RTLD_NEXT, "fclose");
	int fd = fileno(stream);
	file_buf* fb = get_fb_by_fd(fd);
	flush_buf(fb);
	delete_fb(fb);
	return orig_fclose(stream); 
}

int open(const char *filename, int flags, ...){
	int (*orig_open)(const char*, int) = dlsym(RTLD_NEXT,"open");
	return orig_open(filename,flags);
}

ssize_t read(int fd, void *buf, size_t count){
	ssize_t (*orig_read)(int, void*, size_t) = dlsym(RTLD_NEXT, "read");
	return orig_read(fd, buf, count);
}

ssize_t write(int fd, const void *buf, size_t count){
	ssize_t (*orig_write)(int, const void*, size_t) = dlsym(RTLD_NEXT, "write");

	file_buf* fb = get_fb_by_fd(fd);

	if(treat_as_normal(buf, fb)){
	   return orig_write(fd, buf, count-sizeof(NO_INTERCEPT_FLAG));
	}
	else{
		int tmp = append_write(fb, buf, count);
		if(tmp == -1){ // write too big for buffer
			printf("data too large. data: \"%s\". max size: %i\n", (const char*)buf, MAX_BUF_SIZE);
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

bool treat_as_normal(const void* buf, file_buf* fb){
	const char* tmp = (const char*) buf;
	int flagsize = sizeof(NO_INTERCEPT_FLAG);
	char flag[flagsize];
	memcpy(flag, &tmp[fb->curr_size], flagsize);
	if(strcmp(flag, NO_INTERCEPT_FLAG) == 0){	
		return true;
	}
	return false;
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

//debug functions
int main(){
	file_buf* fb = (file_buf*) malloc(sizeof(file_buf));
	fb->filename = "";
	fb->mode = "";
	fb->fd = 0;
	fb->curr_size = 0;
	fb->write_buf = (unsigned char*)malloc(
		sizeof(unsigned char) * (MAX_BUF_SIZE + sizeof(NO_INTERCEPT_FLAG)+1)); 
	fb->next = NULL;

	flush_buf(fb);
	delete_fb(fb);

}

