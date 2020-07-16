#include <stdio.h>
#include <string.h>

int main(int argc, char** argv){

   FILE *fp;
   char str[10] = "1 ";
   char str2[10] = "2 ";
   char str3[10] = "3 ";
   char str4[10] = "4 ";
   char str5[10] = "5 ";

   fp = fopen( "file.txt" , "w" );
   fwrite(str , sizeof(char) , sizeof(str) , fp );
   fwrite(str2 , sizeof(char) , sizeof(str2) , fp );
   fwrite(str3 , sizeof(char) , sizeof(str3) , fp );
   fwrite(str4 , sizeof(char) , sizeof(str4) , fp );
   fwrite(str5 , sizeof(char) , sizeof(str4) , fp );

   fclose(fp);
  
   return(0);
}
