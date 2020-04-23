#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <errno.h> 
#include <unistd.h>


int main(int argc, char **argv){
	char str[] = "yes";

	/*char buf[1024];
	FILE *fp1 = fopen(argv[1], "ab+");
   fwrite(str , 1 , sizeof(str) , fp1);
	fread(buf, 1024, 1, fp1);
	*/
	char buf2[1024];
	int fd1 = open("foo.txt", O_CREAT | O_RDWR);
	write(fd1, str, sizeof(str));
	read(fd1, buf2, 1024);
	
	return 0;
}
