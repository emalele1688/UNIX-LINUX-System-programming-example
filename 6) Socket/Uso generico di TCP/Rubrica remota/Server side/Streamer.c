#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include "Streamer.h"


//TODO: RIVEDERE GESTIONE TIMEOUT

size_t readToStream(int ds_file, void* destBuff, size_t length)
{
  //Byte totali fin ora letti
  size_t byteRead = 0;
  //Byte tornati da una singola operazione di read
  ssize_t byteRet;
  
  while(byteRead < length)
  {
    byteRet = read(ds_file, destBuff, (length-byteRead));
    
    if(byteRet == -1)
    {
      if(errno == EINTR)
	continue;
      else
	return byteRead;
    }
    
    if(byteRet == 0)	//EOF
      return byteRead;
    
    destBuff += byteRet;
    byteRead += byteRet;
  }
  
  return byteRead;
}

size_t writeToStream(int ds, const void* sorgBuff, size_t length)
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
