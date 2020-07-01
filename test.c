#include <stdio.h>
#include <string.h>

int main(int argc, char** argv){

   FILE *fp;
   char str[10] = "This ";
   char str2[10] = "is ";
   char str3[10] = "a test.\n";

   fp = fopen( "file.txt" , "w" );
   fwrite(str , sizeof(char) , sizeof(str) , fp );
   fwrite(str2 , sizeof(char) , sizeof(str2) , fp );
   fwrite(str3 , sizeof(char) , sizeof(str3) , fp );

   fclose(fp);
  
   return(0);
}
