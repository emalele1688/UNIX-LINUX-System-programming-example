#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

#define STRING_SIZE	256

int writeToFile(int ds, void* sorgBuff, size_t length)
{
  int ret;
  size_t Twrite = 0;
  while(Twrite < length)
  {
      ret = write(ds, sorgBuff, (length - Twrite));
      if(ret == -1)
      {
	if(errno == EINTR)
	  continue;
	else
	  return -1;
      }
      
      Twrite += ret;
      sorgBuff += ret;
  }
  
  return Twrite;
}

void load(int ds_file)
{
  printf("Carica lista\n");
  char mess[STRING_SIZE];
  do
  {
    fgets(mess, STRING_SIZE, stdin);
    writeToFile(ds_file, mess, strlen(mess));
  }
  while(strncmp(mess, "quit", 4));
  sync();	
}

void readMap(void* pBuffer, int count)
{
   printf("%s\n", (char*)pBuffer);
}

int main(int argn, char* args[])
{
  struct stat fileInf;
  void* pbuffer;
  int ds_file = open("file.txt", O_RDWR|O_APPEND);
  if(ds_file == -1)
  {
    ds_file = open("file.txt", O_CREAT|O_RDWR, 0660);
    if(ds_file == -1)
    {
      printf("%s\n", strerror(errno));
      return 1;
    }
  }
  load(ds_file);
  
  
  fstat(ds_file, &fileInf);
  printf("Dimensione file %u\n", fileInf.st_size);
  pbuffer = mmap(NULL, fileInf.st_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, ds_file, 0);
  if(pbuffer == -1)
  {
    printf("%s\n", strerror(errno));
    return 1;
  }
  
  readMap(pbuffer, fileInf.st_size);
  
  if(munmap(pbuffer, fileInf.st_size) == -1)
    printf("%s\n", strerror(errno));
  
  if(close(ds_file))
    printf("%s\n", strerror(errno));
  
  return 0;
}