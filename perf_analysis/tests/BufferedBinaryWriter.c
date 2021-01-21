#include "BufferedBinaryWriter.h"
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include "quicklz.h"
#include "initializer.h"
#include "server_common.h"
#include <sys/time.h>
#include <sys/resource.h>

FILE *theFile;
unsigned char * buffptr ;  // moving buffer pointer
unsigned char * buffstart ; //always points to the beginning of the buffer

extern char *my_ckpt_name;
extern char *output_dir_name;

pthread_mutex_t mutex_buffer;
pthread_mutex_t mutex_file;
pthread_cond_t buffer_threshold_cv;
int flush_buffer_flag = 0;
int close_file_flag = 0;
int debug_num_of_times_written = 0;
unsigned char *cache_buffer;//[BUFFERSIZE+2];  //to hold the data so that we can write in chunks
unsigned char *qlz_dst2;
unsigned char *scratch2;

long long int debug_bufwrite_total_data_to_write;
long long int debug_bufwrite_total_data_written;
double gzip_compression_time;
double pfs_xfer_time;
double single_compressed_size;
double double_compressed_size;

FILE * openFile()
{
	/* Initialize mutex and condition variable objects */
	pthread_mutex_init(&mutex_buffer, NULL);
	pthread_mutex_init(&mutex_file, NULL);
	pthread_cond_init (&buffer_threshold_cv, NULL);
	cache_buffer = (unsigned char*)malloc(sizeof(unsigned char) * (BUFFERSIZE + 2));
	bufferedBinaryWriterInit();
	char *tmp_file_name = (char*)malloc(sizeof(char) * (strlen(my_ckpt_name) + 100));
	sprintf(tmp_file_name, "/tmp/%s", my_ckpt_name);
	theFile = fopen(tmp_file_name, "wb");
	free(tmp_file_name);
	qlz_dst2 = (unsigned char*)malloc(sizeof(unsigned char) * BUFFERSIZE);
	scratch2 = (unsigned char*)malloc(sizeof(unsigned char) * BUFFERSIZE);
	//	my_ckpt_name = filename;
	//	output_dir_name = output_dir;
	return theFile;
}

void closeFile()
{
	// mutex unlocked.
	printf("~~~~~~~~~~~~~~~closing file: lock mutex...... \n");
	pthread_mutex_lock(&mutex_buffer);
	flush_buffer_flag = 1;
	close_file_flag = 1;
	pthread_cond_signal(&buffer_threshold_cv);
	printf("~~~~~~~~~~~~~~~closing file, unlock mutex\n");
	pthread_mutex_unlock(&mutex_buffer);

}
// Thread worlk
void *remote_file_write(void *t){
	printf("start local_write 3 %lf\n", now() );

	while(close_file_flag == 0){
		pthread_mutex_lock(&mutex_buffer);
		if(DEBUG)printf("========== flush buffer, lock mutex\n");
		while(flush_buffer_flag == 0){
			if(DEBUG)printf("========== going to wait on lock\n");
			pthread_cond_wait(&buffer_threshold_cv, &mutex_buffer);
		}
		if(DEBUG)printf("========== going to flush data\n");
		if(flush_buffer_flag == 1){
			internal_flushBuffer();
			flush_buffer_flag = 0;
		}

		pthread_mutex_unlock(&mutex_buffer);
	}
	if(DEBUG)printf("========== finishing remote file write....\n");
	internal_flushBuffer();
	fclose(theFile);
	theFile = NULL;
	free(cache_buffer);
	buffptr = NULL;
	buffstart = NULL;
	pthread_mutex_destroy(&mutex_buffer);
	pthread_mutex_destroy(&mutex_file);
	pthread_cond_destroy(&buffer_threshold_cv);

	printf("start copy_pgz 3 %lf\n", now() );
	//tear_down_connection();
	// now, compress the checkpoint and write to the PFS
	char *compressed_ckpt_name;
	compressed_ckpt_name = (char*)malloc(sizeof(char) * (strlen(my_ckpt_name) + 100 + 1));
	//	strcpy(compressed_ckpt_name, my_ckpt_name);
	//	strcat(compressed_ckpt_name, ".gz");
	sprintf(compressed_ckpt_name, "/tmp/%s.pgz", my_ckpt_name);
	char *command;
	command = (char*)malloc(sizeof(char) * (strlen(output_dir_name) + strlen(my_ckpt_name) * 2 + 100 + 1));
	sprintf(command, "cp /p/lscratchc/islam3/framework/framework-test/ionc/pigz-2.2.3/pigz /tmp/");
	system(command); //--- open this comment.

	printf("end copy_pgz 3 %lf\n", now() );


	printf("start parallel_gzip 3 %lf\n", now() );
	///p/lscratchc/islam3/framework/framework-test/ionc
	sprintf(command, "/tmp/pigz -c -f /tmp/%s > %s", my_ckpt_name, compressed_ckpt_name);

//	strcpy(command, "gzip -c /tmp/");
//	strcat(command, my_ckpt_name);
//	strcat(command, " > ");
//	strcat(command, compressed_ckpt_name);
	printf("GZIP: Command: %s\n", command);

	double start_time = now();
	system(command); //-- open this command
	double end_time = now();
	double comp_time = (end_time - start_time);
	gzip_compression_time = comp_time;
	printf("end parallel_gzip 3 %lf\n", now() );



	// Measure the size of this file:
	start_time = now();
	long long int file_size = 0;
	FILE *fptr3 = fopen(compressed_ckpt_name, "rb");
	fseek(fptr3, 0L, SEEK_END);
	file_size = (long long int)ftell(fptr3);
	fclose(fptr3);
	end_time = now();
	printf("prof_find_compressed_file_size_time = %lld B (%lf sec)\n", file_size, (end_time - start_time));

	printf("start pfs_xfer 3 %lf\n", now() );

	sprintf(command, "mv %s %s", compressed_ckpt_name, output_dir_name);
	printf("PFS-Xfer: Command: %s\n", command);
	start_time = now();
	system(command); //-- PFS xfer open this comment
	end_time = now();
	//printf("prof_pfs_xfer_time = %lf\n", (end_time - start_time));
	pfs_xfer_time = (end_time - start_time);
	printf("end pfs_xfer 3 %lf\n", now() );

	//printf("prof_compression_time(GZIP) = %lf\n", comp_time);

//	printf("========== Compression-Size Report   ======\n");
	printf("prof_single_compressed_size %lld\n", debug_bufwrite_total_data_written);
	single_compressed_size = debug_bufwrite_total_data_written;
	double_compressed_size = file_size;
	//	printf("prof_bufferedLZ_size  = %lld\n", debug_bufwrite_total_data_written);
	//	printf("prof_bufferedLZ_Gzip_compressed_size = %lld\n", file_size);

	printf("prof_double_compressed_size %lld\n", file_size);
	printf("end local_write 3 %lf\n", now() );

	if(qlz_dst2 != NULL)
		free(qlz_dst2);
	if(scratch2 != NULL)
		free(scratch2);
	if(command != NULL)
		free(command);
	if(compressed_ckpt_name != NULL)
		free(compressed_ckpt_name);
	pthread_exit(NULL);
}

void flushBuffer(){
	flush_buffer_flag = 1;

	pthread_cond_signal(&buffer_threshold_cv);
	if(DEBUG)printf("============ flush buffer, unlock mutex\n");
	pthread_mutex_unlock(&mutex_buffer);

	//	sleep(1);
}

void writeToBuffer(void *b, unsigned long long size){
	double start_time, end_time;
	int flag =0;
	pthread_mutex_lock(&mutex_buffer);
	start_time = now();
	if(DEBUG)printf("========== writeToBuffer> (%d) lock mutex: %lld\n", debug_num_of_times_written,size);
	if(size)
	{
		debug_num_of_times_written++;
		debug_bufwrite_total_data_to_write += size;
		memcpy((void*)buffptr,b,(size_t)size);
		buffptr+=size;
		if(DEBUG)printf("========== <BufferedBinaryWriter> (%d) size <= BufferSize: %llu, %llu\n",
				debug_num_of_times_written, size, (buffptr - buffstart));
	}
	if((buffptr - buffstart) > (BUFFERSIZE/ 2)){
		flush_buffer_flag = 1;
		pthread_cond_signal(&buffer_threshold_cv);
	}

	if(DEBUG)printf("========== writeToBuffer> (%d) unlock mutex\n", debug_num_of_times_written);
	pthread_mutex_unlock(&mutex_buffer);


}
void writeToBuffer_old(void *b, unsigned long long size)
{
	int flag =0;
	pthread_mutex_lock(&mutex_buffer);
	if(DEBUG)printf("========== writeToBuffer> (%d) lock mutex: %lld\n", debug_num_of_times_written,size);
	if((buffptr + size) >= (buffstart+BUFFERSIZE)){
		flag++;
		if(DEBUG)printf("========== ==== sending signal to flushBuffer thread!!!! \n");
		//flushBuffer();
		flush_buffer_flag = 1;

		pthread_cond_signal(&buffer_threshold_cv);
		if(DEBUG)printf("============ (%d) flush buffer, unlock mutex\n",
				debug_num_of_times_written);
		pthread_mutex_unlock(&mutex_buffer);
	}

	// if size if ever greater than BUFFERSIZE itself, then break it up
	//	unsigned long long i;
	if(flag == 1){
		pthread_mutex_lock(&mutex_buffer);
		flag = 0;
	}
	//	while(size > BUFFERSIZE)
	//	{
	//		memcpy((void*)buffptr,b,(size_t)BUFFERSIZE);
	//		buffptr+=BUFFERSIZE;
	//		flag++;
	//		flushBuffer();
	//
	//		if(flag == 1){
	//			pthread_mutex_lock(&mutex_buffer);
	//			flag = 0;
	//		}
	//
	//		b += BUFFERSIZE;
	//		size -= BUFFERSIZE;
	//		if(DEBUG)printf("<BufferedBinaryWriter> size > BufferSize: %llu, %llu\n", size, (buffptr - buffstart));
	//	}
	if(size)
	{
		debug_num_of_times_written++;
		debug_bufwrite_total_data_to_write += size;
		memcpy((void*)buffptr,b,(size_t)size);
		buffptr+=size;
		printf("========== <BufferedBinaryWriter> (%d) size <= BufferSize: %llu, %llu\n",
				debug_num_of_times_written, size, (buffptr - buffstart));
	}
	//	end_time = now();
	//	global_localDiskWrite_time += (end_time - start_time);
	if((buffptr - buffstart) > (BUFFERSIZE / 2)){
		//	flushBuffer();// this guy unlocks the mutex anywats
		flush_buffer_flag = 1;
		pthread_cond_signal(&buffer_threshold_cv);
	}

	if(DEBUG)printf("========== writeToBuffer> (%d) unlock mutex\n", debug_num_of_times_written);
	pthread_mutex_unlock(&mutex_buffer);

}

// with gzip: too slow
void internal_flushBuffer_with_GZIP(){
	long long int len;
	if((buffptr - buffstart) > 0 ){
		//debug_bufwrite_total_data_written += fwrite((unsigned char*)buffstart,sizeof(unsigned char),buffptr-buffstart,theFile);
		len = qlz_compress((unsigned char*)buffstart, qlz_dst2, (sizeof(unsigned char) * (buffptr-buffstart)), scratch2);
		fwrite(qlz_dst2, sizeof(unsigned char), len, theFile);
		debug_bufwrite_total_data_written += len;
	}
	buffptr=buffstart;

}

void internal_flushBuffer()
{
	// ProcessTime example
	struct timeval startTime;
	struct timeval endTime;
	//structure for rusage
	struct rusage ru;
	// get the current time
	// - RUSAGE_SELF for current process
	// - RUSAGE_CHILDREN for *terminated* subprocesses
	getrusage(RUSAGE_SELF, &ru);
	startTime = ru.ru_utime;

	//	globalTotalDataWritten += fwrite((unsigned char*)buffstart,sizeof(unsigned char),buffptr-buffstart,theFile);
	if((buffptr - buffstart) > 0 ){
		debug_bufwrite_total_data_written += fwrite((unsigned char*)buffstart,sizeof(unsigned char),buffptr-buffstart,theFile);
	}
	//	if(DEBUG)printf("BUFFERWRITE> %llu\n", (buffptr-buffstart));
	//	global_localDiskWrite_time += (end_time - start_time);
	buffptr=buffstart;

	// get the end time
	getrusage(RUSAGE_SELF, &ru);
	endTime = ru.ru_utime;
	// calculate time in microseconds
	double tS = startTime.tv_sec*1000000 + (startTime.tv_usec);
	double tE = endTime.tv_sec*1000000  + (endTime.tv_usec);
	update_buffer_write_cpu_time( (tE - tS) );

}

void bufferedBinaryWriterInit()
{
	buffptr = cache_buffer;
	buffstart = cache_buffer;
	//	theFile = NULL;
}
