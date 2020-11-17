#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <stdbool.h> 
#include <sys/time.h>
#include <pthread.h>
#include "filcio.h"

// 0: none | 1: write function timers to file | 2: errors
#define DEBUG_LVL 0 

void* flush_handler(void* args){

	while(1){
		pthread_mutex_lock(&mutex_flush);
		pthread_cond_wait(&cond_flush, &mutex_flush);
		
		pthread_rwlock_wrlock(&rwlock);
		int tmp = write(wa.fd, wa.buf, wa.size);
		pthread_rwlock_unlock(&rwlock);
	
		pthread_mutex_unlock(&mutex_flush);
	}
}

void flush_buf(file_buf* fb){

	struct timeval begin, end;
	gettimeofday(&begin, 0);

	if(!FLUSHER){
		pthread_mutex_init(&mutex_flush, NULL);
		pthread_cond_init(&cond_flush, NULL);
		pthread_mutex_init(&mutex_wa, NULL);
		pthread_create(&FLUSHER, NULL, &flush_handler, NULL);
	}

	// data about this flush placed in global buffer for FLUSHER to read.
	pthread_mutex_lock(&mutex_wa);
		wa.fd   = -1*fb->fd; //negative fd signals this is a normal write.
		wa.buf  = fb->write_buf;
		wa.size = fb->curr_size;
		pthread_cond_signal(&cond_flush);
	pthread_mutex_unlock(&mutex_wa);

	record_wallclock(begin, end, fb, "flush");
	struct timeval begin2, end2;
	gettimeofday(&begin2, 0);

	memset(fb->write_buf, 0, fb->curr_size);
	fb->curr_size = 0;

	record_wallclock(begin2, end2, fb, "flush_memset");

}

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

int append_write(file_buf* fb, const void* buf, size_t size){
	
	// wall-clock timing {
		struct timeval begin, end;
		gettimeofday(&begin, 0);
	// }

	int retval = -1;
	if(size <= GLOBAL_BUF_SIZE){
		if(fb->curr_size + size > GLOBAL_BUF_SIZE){
			if(DEBUG_LVL>=2){printf("not enough space remaining. data size: %li. curr_size: %li. max size: %i\n", size, fb->curr_size, GLOBAL_BUF_SIZE);}
			flush_buf(fb);
		}
		
		memcpy(&fb->write_buf[fb->curr_size], buf, size);
		fb->curr_size += size;

		retval = 0;
	}

	record_wallclock(begin, end, fb, "append_write");

	return retval;
}

void insert_fb(int fd, const char* filename, const char* mode){

	struct timeval begin, end;
	gettimeofday(&begin, 0);

	//earliest possible spot to grab the env variable
	char* tmp = getenv(BUF_SIZE_ENV_VAR);
	if(tmp!=NULL){
		GLOBAL_BUF_SIZE = atoi(tmp);
	}

	if(get_fb_by_fd(fd) != NULL){ //already exists
		if(DEBUG_LVL>=2){printf("Error: this file buffer is already in memory");}
	}
	else{
		file_buf* old_head = global_fb_ptr;
		file_buf* new_fb = (file_buf*) malloc(sizeof(file_buf));
		new_fb->filename = filename;
		new_fb->mode = mode;
		new_fb->fd = fd;
		new_fb->curr_size = 0;
		new_fb->write_buf = (unsigned char*)malloc(
			sizeof(unsigned char) * (GLOBAL_BUF_SIZE+1)); 
		new_fb->next = old_head;
		global_fb_ptr = new_fb; 

		record_wallclock(begin, end, new_fb, "insert_fb");
	}

}


void delete_fb(file_buf* fb){
	free(fb->write_buf);
	free(fb);
}


/* ===== INTERCEPTION ===== */

FILE* fopen(const char *filename, const char *mode){
	FILE* (*orig_fopen)(const char*, const char*) = dlsym(RTLD_NEXT, "fopen");

	//check if this is a debug file for this library
	if(mode[0] == '!'){
		FILE* orig_retval = orig_fopen(filename, &mode[1]);
		return orig_retval;
	}
	
	FILE* orig_retval = orig_fopen(filename, mode);
	insert_fb(fileno(orig_retval), filename, mode);
	return orig_retval;
}

size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream){

	// wall-clock timing {
		struct timeval begin, end;
		gettimeofday(&begin, 0);
	// }

	if(!orig_fwrite){ orig_fwrite = dlsym(RTLD_NEXT, "fwrite"); }

	file_buf* fb = get_fb_by_fd(fileno(stream));
	
	int too_big_flag = append_write(fb, ptr, nmemb);
	if(too_big_flag == -1){ // write too big for buffer
	   return orig_fwrite(ptr, size, nmemb, stream);	  	 
	}

	record_wallclock(begin, end, fb, "fwrite");	

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
	if(fb){
		flush_buf(fb);
		delete_fb(fb);
	}
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


void record_wallclock(struct timeval begin, struct timeval end, file_buf* fb, char* name){
	if(DEBUG_LVL>=1){
		gettimeofday(&end, NULL);
		long seconds = end.tv_sec - begin.tv_sec;
		long microseconds = end.tv_usec - begin.tv_usec;
		double elapsed = seconds + microseconds*1e-6;
		double mbytes_per_sec = (fb->curr_size/elapsed)/1000000;
		//printf("Time to flush %ld bytes: %.3f seconds. (%.3f MB/sec) \n", fb->curr_size, elapsed, mbytes_per_sec);//human readable
		FILE* f = fopen("function_timers.log", "!a");
		fprintf(f, "%s, %.6f\n", name, elapsed);
		fclose(f);
	}
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


