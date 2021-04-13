#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include "filcio.h"


void* flush_handler(void* args){

	while(1){
		pthread_mutex_lock(&mutex_flush);

		pthread_cond_wait(&cond_flush, &mutex_flush);

		pthread_rwlock_wrlock(&flushlock);
			int tmp = write(fa.fd, fa.buf, fa.size);
		pthread_rwlock_unlock(&flushlock);
	
		pthread_mutex_unlock(&mutex_flush);
	}
}

void flush_buf(file_buf* fb){
	
	struct timeval begin, end;
	gettimeofday(&begin, 0);

	if(!FLUSHER){
		
		pthread_mutex_init(&mutex_flush, NULL);
		pthread_cond_init(&cond_flush, NULL);
		pthread_mutex_init(&mutex_fa, NULL);

		pthread_create(&FLUSHER, NULL, &flush_handler, NULL);
	}

	// data about this flush placed in global buffer for FLUSHER to read.
	pthread_mutex_lock(&mutex_fa);
		fa.fd   = -1*fb->fd; //negative fd signals this is a normal write.
		fa.buf  = fb->buf;
		fa.size = fb->curr_size;
		pthread_cond_signal(&cond_flush);
	pthread_mutex_unlock(&mutex_fa);

	record_wallclock(begin, end, fb, "flush");
	struct timeval begin2, end2;
	gettimeofday(&begin2, 0);

	//switch which buffer is being filled
	if(fb->buf == fb->bufA)
		fb->buf = fb->bufB;
	else
		fb->buf = fb->bufA;

	fb->curr_size = 0;
	fb->flushes++;

	record_wallclock(begin2, end2, fb, "flush_memset");
}


void final_flush(file_buf* fb){
	if(DEBUG_LVL>=2){printf("Final flush\n");}
	fb->flushes++;

	pthread_rwlock_wrlock(&flushlock);
	write(-(fb->fd), fb->buf, fb->curr_size);
	pthread_rwlock_unlock(&flushlock);
}

void* memcpy_handler(void* args){

	while(1){
		pthread_mutex_lock(&mutex_memcpy);

		pthread_cond_wait(&cond_memcpy, &mutex_memcpy);
		
		pthread_rwlock_wrlock(&memcpylock);
			memcpy(&mca.fb->buf[mca.fb->curr_size], mca.buf, mca.size);
			mca.fb->curr_size += mca.size;
			mca.fb->attempted_writes++;
			mca.fb->total_data_int += mca.size;
		pthread_rwlock_unlock(&memcpylock);
	
		pthread_mutex_unlock(&mutex_memcpy);
	}
}

int append_write(file_buf* fb, const void* buf, size_t size){
	if(DEBUG_LVL>=2){printf("copying write to memory\n");}
	if(PASS_AGG){return 1;}
	
	struct timeval begin, end;
	gettimeofday(&begin, 0);

	int retval = 1;
	if(size <= GLOBAL_BUF_SIZE){
		if(fb->curr_size + size > GLOBAL_BUF_SIZE){
			flush_buf(fb);
		}
		
		if(!MEMCPYR){
			pthread_mutex_init(&mutex_memcpy, NULL);
			pthread_cond_init( &cond_memcpy,  NULL);
			pthread_mutex_init(&mutex_mca,    NULL);
			pthread_create(&MEMCPYR, NULL, &memcpy_handler, NULL);
		}
			// data about this memcpy placed in global buffer for MEMCPYR to read.
			pthread_mutex_lock(&mutex_mca);
				mca.fb  = fb;
				mca.buf  = buf;
				mca.size = size;
				pthread_cond_signal(&cond_memcpy);
			pthread_mutex_unlock(&mutex_mca);

			retval = 0;

		}

	record_wallclock(begin, end, fb, "append_write");
	return retval;
}


file_buf* get_fb_by_fd(int fd){
	file_buf* tmp = head;
	while(tmp != NULL){
		if(tmp->fd == fd){
			return tmp;
		}
		tmp = tmp->next;
	}
	return tmp;
}


file_buf* alloc_fb(int fd, const char* filename, const char* mode){

	struct timeval begin, end;
	gettimeofday(&begin, 0);

	//earliest possible spot to grab the env variables
	char* tmp = getenv(BUF_SIZE_ENV_VAR);
	if(tmp!=NULL){ GLOBAL_BUF_SIZE = atoi(tmp); }
	tmp = getenv(DEBUG_LVL_ENV_VAR);
	if(tmp!=NULL){ DEBUG_LVL = atoi(tmp); }
	tmp = getenv(PASS_AGG_ENV_VAR);
	if(tmp!=NULL){ PASS_AGG = atoi(tmp); }

	if(DEBUG_LVL>=2){printf("Allocating memory for new file buf\n");}

	file_buf* new_fb = (file_buf*) malloc(sizeof(file_buf));

	if(get_fb_by_fd(fd) != NULL){ 
		if(DEBUG_LVL>=2){printf("Error: this file buffer is already in memory\n");}
	}
	else{
		new_fb->bufA = (unsigned char*)malloc(
			sizeof(unsigned char) * (GLOBAL_BUF_SIZE+1)); 
		new_fb->bufB = (unsigned char*)malloc(
			sizeof(unsigned char) * (GLOBAL_BUF_SIZE+1)); 
		file_buf* old_head       = head;
		new_fb->filename         = filename;
		new_fb->mode             = mode;
		new_fb->fd               = fd;
		new_fb->curr_size        = 0;
		new_fb->buf              = new_fb->bufA;
		new_fb->attempted_writes = 0;
		new_fb->flushes          = 0;
		new_fb->total_data_int   = 0;
		new_fb->next             = old_head;
		head                     = new_fb; 

		record_wallclock(begin, end, new_fb, "alloc_fb");
	}
	return new_fb;
}


void delete_fb(file_buf* fb){
	file_buf *temp = head, *prev;
	if(DEBUG_LVL>=1){
		printf("File: %s\nAttempted writes: %i\nFlushes: %i\n",
		       fb->filename, fb->attempted_writes, fb->flushes);
		if(fb->attempted_writes > 0){
			printf("Avg size: %d\n", fb->total_data_int / fb->attempted_writes);
		}
	}

	if (temp != NULL && temp == fb) {
		head = temp->next;
		free(temp);
		return;
	}
	while (temp != NULL && temp != fb) {
		prev = temp;
		temp = temp->next;
	}
	if (temp == NULL) 
		return;

  prev->next = temp->next;
  free(temp);
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
	alloc_fb(fileno(orig_retval), filename, mode);
	return orig_retval;
}

size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream){

	struct timeval begin, end;
	gettimeofday(&begin, 0);

	if(!orig_fwrite){ orig_fwrite = dlsym(RTLD_NEXT, "fwrite"); }

	file_buf* fb = get_fb_by_fd(fileno(stream));
	if(!fb){ 
		fb = alloc_fb(12, "filename", "a"); 
	}
	
	int too_big = append_write(fb, ptr, nmemb);
	if(too_big){ 
	   return orig_fwrite(ptr, size, nmemb, stream);	  	 
	}

	record_wallclock(begin, end, fb, "fwrite");	
	return nmemb;//orig retval
}


ssize_t write(int fd, const void *buf, size_t count){
	ssize_t (*orig_write)(int, const void*, size_t) = dlsym(RTLD_NEXT, "write");

	file_buf* fb = get_fb_by_fd(fd);

	//negative fd signals this is a normal write.
	if(fd<0){
	   return orig_write(-fd, buf, count);
	}
	else{
		int too_big = append_write(fb, buf, count);
		if(too_big){ 
			return orig_write(fd, buf, count);
		}
	}
	return count;//orig retval
}


int fclose(FILE* stream){
	int (*orig_fclose)(FILE*) = dlsym(RTLD_NEXT, "fclose");
	int fd = fileno(stream);
	file_buf* fb = get_fb_by_fd(fd);
	if(fb){
		final_flush(fb);
		delete_fb(fb);
	}
		return orig_fclose(stream); 
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream){
	size_t (*orig_fread)(void *, size_t, size_t, FILE*) = dlsym(RTLD_NEXT, "fread");
	int fd = fileno(stream);
	file_buf* fb = get_fb_by_fd(fd);
	if(fb){final_flush(fb);}
	return orig_fread(ptr, size, nmemb, stream);
}

int open(const char *filename, int flags, ...){
	printf("open called\n");
	int (*orig_open)(const char*, int) = dlsym(RTLD_NEXT,"open");
	int orig_retval = orig_open(filename, flags);
	alloc_fb(orig_retval, filename, "");
	return orig_retval;
}

ssize_t read(int fd, void *buf, size_t count){
	ssize_t (*orig_read)(int, void*, size_t) = dlsym(RTLD_NEXT, "read");
	file_buf* fb = get_fb_by_fd(fd);
	if(fb){final_flush(fb);}
	return orig_read(fd, buf, count);
}

/* TODO: this needs its own locking. reusing the same from fclose causes undefined behavior */
int close(int fd){
	int (*orig_close)(int) = dlsym(RTLD_NEXT, "close");
	file_buf* fb = get_fb_by_fd(fd);
	if(fb){
		flush_buf(fb);
		delete_fb(fb);
	}
	return orig_close(fd); 
}

/*
size_t fprintf(FILE *stream, const char *format, ...){
	if(!orig_fprintf){ orig_fwrite = dlsym(RTLD_NEXT, "fprintf"); }
	file_buf* fb = get_fb_by_fd(fileno(stream));
	int too_big = append_write(fb, ?);
	if(too_big){ 
	   return orig_fprintf(stream, format);	  	 
	}
	return ?;
}
*/

void record_wallclock(struct timeval begin, struct timeval end, file_buf* fb, char* name){
	if(DEBUG_LVL>=2){
		gettimeofday(&end, NULL);
		long seconds = end.tv_sec - begin.tv_sec;
		long microseconds = end.tv_usec - begin.tv_usec;
		double elapsed = seconds + microseconds*1e-6;
		double mbytes_per_sec = (fb->curr_size/elapsed)/1000000;
		FILE* f = fopen("function_timers.log", "!a");
		fprintf(f, "%s, %.6f\n", name, elapsed);
		fclose(f);
	}
}

