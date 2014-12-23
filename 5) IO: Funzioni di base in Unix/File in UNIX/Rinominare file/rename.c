#include <stdio.h>

int main(int arg, char *argv[])
{
  if(arg < 3)
  {
    printf("Parametri mancanti: vecchio_nome nuovo_nome\n");
    exit(1);
  }
  
   if( link(argv[1], argv[2]) == -1 )
   {
     printf("File %s o %s non esistenti\n", argv[1], argv[2]);
     exit(1);
   }
   
   if( unlink(argv[1]) == -1 )
   {
     perror("unlink");
     exit(1);
   }
   
   return 0;
}
