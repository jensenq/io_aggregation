#ifndef FILCIO_H
#define FILCIO_H

#define _GNU_SOURCE
#define BUF_SIZE_ENV_VAR "AGG_BUFSIZE"

typedef struct file_buf{
	const char* filename;
	const char* mode;
	int fd;
	size_t curr_size;         // how full the buffer is in bytes
	unsigned char* write_buf; // one contiguous chunk of memory
	struct file_buf* next;    // this is a linked list
} file_buf;

//upon flushing, main thread writes arguments here, FLUSHER reads these args
typedef struct write_args{
	int fd;
	void* buf;
	size_t size;
} write_args;

//

file_buf* global_fb_ptr = NULL;
int GLOBAL_BUF_SIZE = 32000000; //default 32MB

pthread_cond_t   cond_flush;  // flushing signal
pthread_mutex_t  mutex_flush; // ^

write_args       wa;          // data about a flush
pthread_mutex_t  mutex_wa;    // protects wa
pthread_t        FLUSHER;     // single thread waits for signal.
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

//

/* ======== Docs and Defs ======== */

/* spins FLUSHER if it's not spun yet (aka this is the first flush). Then passes 
   data about this flush to the global variable, wa, and signals FLUSHER to flush.
   finall, resets the file_buf's buffer */
void flush_buf(file_buf*);

/* upon receiving the signal that it's time to flush, reads the global variable, 
   wa, then flushes. This (hopefully) allows the rest of the program to carry on 
   without blocking. */
void* flush_handler(void*);

/* copies the buffer of a write() into our write buffer
 * if this new data will overflow the buffer, flush the buffer first.
 * returns 0 if successful, -1 if the write is too large for the buffer,
 * which should be written normally
 */
int append_write(file_buf*, const void*, size_t);

/* creates a new file buffer, and adds it to the global list of file buffers
 *  note: inserted at the head of the list (files recently opened are more likely to be used soon) 
 */
void insert_fb(int, const char*, const char*);

/* returns a pointer to the file buffer associated with a file descriptor
 */
file_buf* get_fb_by_fd(int);

/* safely removes from the global linked list, frees memory */
void delete_fb(file_buf*);

void record_wallclock(struct timeval, struct timeval, file_buf*, char*);
size_t (*orig_fwrite)(const void*, size_t, size_t, FILE*);

#endif 
