#ifndef FILCIO_H
#define FILCIO_H

#define _GNU_SOURCE
#define BUF_SIZE_ENV_VAR "AGG_BUFSIZE"
#define DEBUG_LVL_ENV_VAR "DEBUG_LVL"
#define PASS_AGG_ENV_VAR "PASS_AGG"


typedef struct file_buf{
	const char* filename;
	const char* mode;
	int fd;
	size_t curr_size;     	// also acts as pointer to the end of data
	unsigned char* buf;  	// buffer currently being filled
	unsigned char* bufA;  	// double buffering implemented to allow one buffer
	unsigned char* bufB;  	// to be filled, while the other is flushed.
	struct file_buf* next;	// this is a linked list
	int attempted_writes;   // total num writes intercepted
	int flushes;            // total num flushes
	int total_data_int;     // total amt of data interecepted
} file_buf;


//upon flushing, main thread writes arguments here, FLUSHER reads these args
typedef struct flush_args{
	int fd;
	void* buf;
	size_t size;
} flush_args;

//approaching a memcpy, main thread writes arguments here, MEMCPYR reads these args
typedef struct memcpy_args{
	file_buf* fb;
	void* buf;
	size_t size;
} memcpy_args;


file_buf* head = NULL;
int GLOBAL_BUF_SIZE = 32000000; //default 32MB
int DEBUG_LVL = 0; 
int PASS_AGG = 0; // if 1: pass aggregating IO, only count


pthread_cond_t   cond_flush;  // flushing signal
pthread_mutex_t  mutex_flush; // ^
flush_args       fa;          // data about a flush
pthread_mutex_t  mutex_fa;    // protects fa
pthread_t        FLUSHER;     // single thread waits for signal.
pthread_rwlock_t flushlock = PTHREAD_RWLOCK_INITIALIZER;

pthread_cond_t   cond_memcpy;  // memcpy signal
pthread_mutex_t  mutex_memcpy; // ^
memcpy_args       mca;          // data about a memcpy
pthread_mutex_t  mutex_mca;    // protects mca
pthread_t        MEMCPYR;     // single thread waits for signal.
pthread_rwlock_t memcpylock = PTHREAD_RWLOCK_INITIALIZER;


//

/* ======== Docs and Defs ======== */

/* spins FLUSHER if it's not spun yet (aka this is the first flush). Then passes 
   data about this flush to the global variable, fa, and signals FLUSHER to flush.
   finally, resets the file_buf's buffer */
void flush_buf(file_buf*);

/* upon receiving the signal that it's time to flush, reads the global variable, 
   fa, then flushes. This (hopefully) allows the rest of the program to carry on 
   without blocking. */
void* flush_handler(void*);

/* special flush for fclose(), so we don't have to deal with race cases */
void final_flush(file_buf*);

/* copies the buffer of a write() into our write buffer
 * if this new data will overflow the buffer, flush the buffer first.
 * returns 0 if successful, 1 if the write is too large for the buffer,
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

/* removes from the global linked list, frees memory. Classic linked list deletion.*/
void delete_fb(file_buf*);

void record_wallclock(struct timeval, struct timeval, file_buf*, char*);
size_t (*orig_fwrite)(const void*, size_t, size_t, FILE*);

/*
b flush_buf
y
b flush_handler
y
b append_write
y
b insert_fb
y
b get_fb_by_fd
y
b delete_fb
y
*/

#endif 
