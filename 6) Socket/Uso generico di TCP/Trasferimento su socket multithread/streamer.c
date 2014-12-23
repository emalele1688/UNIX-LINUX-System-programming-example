#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include "streamer.h"


size_t read_block(int file_descriptor, void *dest_buff, size_t file_size)
{
  //Byte totali fin ora letti
  size_t byteRead = 0;
  //Byte tornati da una singola operazione di read
  ssize_t byteRet;
  
  while(byteRead < file_size)
  {
    byteRet = read(file_descriptor, dest_buff, (file_size-byteRead));
    
    if(byteRet == -1)
    {
      if(errno == EINTR)
	continue;
      else
	return byteRead;
    }
    
    if(byteRet == 0)	//EOF
      return byteRead;
    
    dest_buff += byteRet;
    byteRead += byteRet;
  }
  
  return byteRead;
}

size_t write_block(int ds, const void* sorgBuff, size_t length)
{
  size_t byteWrote = 0;
  ssize_t byteRet;
  while(byteWrote < length)
  {
    byteRet = write(ds, sorgBuff, (length - byteWrote));
    if(byteRet == -1)
    {
      if(errno == EINTR)
	continue;
      else
	return byteWrote;
    }
    
    sorgBuff += byteRet;
    byteWrote += byteRet;
  }
  
  return byteWrote;
}

