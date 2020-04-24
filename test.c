#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <errno.h> 
#include <unistd.h>


int main(int argc, char **argv){
	char* str = "ddd";

	FILE *fp1 = fopen(argv[1], "ab+");
   fwrite(str , 1 , sizeof(str) , fp1);

	char buf[1024];
	fread(buf, 1024, 1, fp1);
	fclose(fp1);
	
	int fd1 = open("foo.txt", O_CREAT | O_RDWR);
	write(fd1, str, sizeof(str));
	char buf2[1024];
	read(fd1, buf2, 1024);
	close(fd1);
	
	return 0;
}
