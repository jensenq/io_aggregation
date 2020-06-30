#include <stdio.h>
#include <string.h>

int main(int argc, char** argv){

   FILE *fp;
   char str[10] = "Thish";

   fp = fopen( "file.txt" , "w" );
   fwrite(str , strlen(str) , sizeof(char) , fp );

   fclose(fp);
  
   return(0);
}
