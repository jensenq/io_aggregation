#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 

/* William Morris
	https://codereview.stackexchange.com/questions/29198/random-string-generator-in-c 
*/
static char *rand_string(char *str, size_t size)
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}
char* rand_string_alloc(size_t size)
{
     char *s = malloc(size + 1);
     if (s) {
         rand_string(s, size);
     }
     return s;
}

int main(int argc, char** argv){

	int NUM_ITERS = 10000;
	int MAX_SIZE = 100000;
	int NUM_FILES = 1;
	if(argc >= 2){
		NUM_ITERS = atoi(argv[1]);
		MAX_SIZE = atoi(argv[2]);
		NUM_FILES = atoi(argv[3]);
	}
	char* rs = rand_string_alloc(MAX_SIZE);
	char fname[32];

	for(int j=0; j<NUM_FILES; j++){
		rand_string(fname,5);
		strcat(fname, ".txt");
   	FILE* fp = fopen( fname , "w" );

		for(int i=0; i<NUM_ITERS; i++){	
			size_t size = rand() % MAX_SIZE;
			rand_string(rs, size);
	   	fwrite(rs, sizeof(char), size, fp );
		}	
		fclose(fp);
	}
	free(rs);
  
   return(0);
}
