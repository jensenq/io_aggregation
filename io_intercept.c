#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

char* recover_filename(int fd);
void log_access(char* fname, char* type, size_t num_bytes);
 
// not necessary if we're recording freads and fwrites
/*int open(const char *filename, int flags, ...){
	int (*orig_open)(const char*, int) = dlsym(RTLD_NEXT,"open");
	log_access((char*)filename, "open", 0);
	return orig_open(filename,flags);
}

FILE* fopen(const char *filename, const char *mode){
	FILE* (*orig_fopen)(const char*, const char*) = dlsym(RTLD_NEXT, "fopen");
	log_access((char*)filename, "open", 0);
	return orig_fopen(filename, mode);
}*/

pid_t fork(){
	pid_t (*orig_fork)() = dlsym(RTLD_NEXT, "fork");
	//printf("fork intercepted");
	return orig_fork();
}

ssize_t read(int fd, void *buf, size_t count){
	ssize_t (*orig_read)(int, void*, size_t) = dlsym(RTLD_NEXT, "read");
	char* fname = recover_filename(fd);
	log_access(fname, "read", count);
	free(fname);
	return orig_read(fd, buf, count);
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream){
	size_t (*orig_fread)(void *, size_t, size_t, FILE*) = dlsym(RTLD_NEXT, "fread");
	char *fname = recover_filename(fileno(stream));
	log_access(fname, "read", size*nmemb);
	free(fname);
	return orig_fread(ptr, size, nmemb, stream);
}

ssize_t write(int fd, const void *buf, size_t count){
	ssize_t (*orig_write)(int, const void*, size_t) = dlsym(RTLD_NEXT, "write");
	char *fname = recover_filename(fd);
	log_access(fname, "write", count);
	free(fname);
	return orig_write(fd, buf, count);
}

size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream){
	size_t (*orig_fwrite)(const void*, size_t, size_t, FILE*) = dlsym(RTLD_NEXT, "fwrite");
	char *fname = recover_filename(fileno(stream));
	log_access(fname, "write", size*nmemb);
	free(fname);
	return orig_fwrite(ptr, size, nmemb, stream);
}

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
	fprintf(f, "%s, %s, %zu\n", fname, type, num_bytes);
	fclose(f);
}



