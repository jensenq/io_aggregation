#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
 

/* William Morris
	https://codereview.stackexchange.com/questions/29198/random-string-generator-in-c 
*/
static char *rand_string(char *str, size_t size){
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];}
        str[size] = '\0';}
    return str;}
char* rand_string_alloc(size_t size){
     char *s = malloc(size + 1);
     if (s) {rand_string(s, size);}
     return s; }

int main(int argc, char** argv){

	int NUM_ITERS = 10000;
	int MAX_SIZE = 100000;
	int NUM_FILES = 1;
	int UNIQUE_STRS = 0;
	if(argc > 4){
		NUM_ITERS = atoi(argv[1]);
		MAX_SIZE = atoi(argv[2]);
		NUM_FILES = atoi(argv[3]);
		//UNIQUE_STRS = atoi(argv[4]);
	}
	 char* rs = rand_string_alloc(MAX_SIZE);
	 struct stat st;
	 char prefix[32] = "/tmp/junk_delete_me/";
	 if (stat(prefix, &st) == -1) {
		  mkdir(prefix, 0700);
	 } 
	if(!UNIQUE_STRS){
		rand_string(rs, MAX_SIZE);	
	}


	 for(int j=0; j<NUM_FILES; j++){
		  char fname[64] = "";
		  strcat(fname, prefix);
		  strcat(fname, rand_string(&fname[strlen(fname)],5));
		  strcat(fname, ".txt");
		  FILE* fp = fopen( fname , "w" );

			for(int i=0; i<NUM_ITERS; i++){	
				if(UNIQUE_STRS){
					size_t size = rand() % MAX_SIZE;
					rand_string(rs, size);	
				}

		  fwrite(rs, sizeof(char), MAX_SIZE, fp );
		}
		fclose(fp);
	}
	free(rs);
  
   return(0);
}
