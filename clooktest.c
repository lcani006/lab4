#include <stdio.h>

int main()
{
   
   
   printf("Reading and Writing to iotest.txt");
   
   
   FILE *file;
   file = (fopen("iotest.txt", "a"));
   if(file == NULL)
   {
       printf("Error!");
       return 1;
   }

    int i = 0;
    int num = 10;
    char buffer[256];
   for(i = 0; i < num; ++i)
   {
      
      fprintf(file,"Test %d \n", i + 1);
      fgets(buffer, 10, file);
   }

   fclose(file);
   return 0;
}