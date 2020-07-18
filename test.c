#include <stdio.h>
#include <string.h>

int main(int argc, char** argv){

   char str[10] = "1 ";
   char str2[10] = "2 ";
   char str3[10] = "3 ";
   char str4[10] = "4 ";
   char str5[10] = "5 ";

	char readbuf[10];

   FILE* fp = fopen( "file.txt" , "w" );
   FILE* fp2 = fopen( "file2.txt" , "w" );

   fwrite(str , sizeof(char) , sizeof(str) , fp );
   fwrite(str2 , sizeof(char) , sizeof(str2) , fp );
	fwrite(str , sizeof(char) , sizeof(str) ,   fp2 );
   fwrite(str3 , sizeof(char) , sizeof(str3) , fp );
   fwrite(str2 , sizeof(char) , sizeof(str2) , fp2 );
   fwrite(str3 , sizeof(char) , sizeof(str3) , fp2 );
   fwrite(str4 , sizeof(char) , sizeof(str4) , fp );
	fread(readbuf, sizeof(char), sizeof(readbuf), fp2);
   fwrite(str4 , sizeof(char) , sizeof(str4) , fp2 );
	fread(readbuf, sizeof(char), sizeof(readbuf), fp);
   fwrite(str5 , sizeof(char) , sizeof(str4) , fp );
	fclose(fp);
   fwrite(str5 , sizeof(char) , sizeof(str4) , fp2 );



   fclose(fp2);
  
   return(0);
}
