#include "TrasfererMan.h"
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

/* Leggi lengthToRead (o di meno) byte dalla socket ds_sock e memorizzali in buffer
 * Torna il numero di byte letti nel buffer altrimenti -1 in caso di errore
 * Se torna un numero di byte inferiore a lengthToRead allora dall'altro capo la ds_sock è chiusa (read = 0) e qualcosa è stato trasferito e messo in buffer
 * Se torna 0 la ds_sock era già chiusa
 */
int receiveData(int ds_sock, void* buffer, size_t lengthToRead)
{
  //Byte letti dal buffer read ad ogni iterazione
  size_t byte_read;
  
  //Totale di byte letti dal buffer read e scaricati in buffer
  size_t mlength = 0;
  
  do
  {
    byte_read = read(ds_sock, buffer, lengthToRead);
    if(byte_read == -1)
    {
      if(errno == EINTR)
	continue;
      printf("receiveData: %s\n", strerror(errno));
      return -1;
    }
    
    //Decremento la quantità di byte ancora da leggere
    lengthToRead -= byte_read;
    
    //Incremento il totale di byte letti dal ds_sock
    mlength += byte_read;
    
    //Scorro il puntatore al buffer in avanti
    buffer += byte_read;
  }
  //Se ancora ho byte da leggere e nell'ultimo ciclo il socket era aperto
  while(lengthToRead > 0 && byte_read != 0);
  
  return mlength;
}


/*Invia sulla ds_sock il messaggio buffer di lunghezza lengthToWrite.
 * Se il messaggio è più lungo di lengthToWrite sarà inviata solo la porzione lengthToWrite
 * Torna il numero di byte inviati o -1 in caso di errore
 */
int sendData(int ds_sock, const void* buffer, size_t lengthToWrite)
{
  //Quantità di byte scritti ad ogni iterazione
  size_t byte_write;
  
  //Quantità di byte scritti nel buffer di write
  size_t mlength = 0;
  
  do
  {
    byte_write = write(ds_sock, buffer, lengthToWrite);
    if(byte_write == -1)
    {
      if(errno == EINTR)
	continue;
      printf("sendData: %s\n", strerror(errno));
      return -1;
    }
    
    //Decrementa la quantità di byte da inviare al ds_sock
    lengthToWrite -= byte_write;
    
    //Aumenta la quantità di byte scritti su buffer
    mlength += byte_write;
    
    //Scorro il puntatore al buffer in avanti
    buffer += byte_write;
  }
  //Se ancora ho byte da scrivere ciclo 
  while(lengthToWrite > 0);
  
  return mlength;
}


/*Riceve un file dalla ds_sock di dimensione massima filesize salvandolo in un file di nome nfile sul disco tornandone il descrittore
 */
int receiveFile(int ds_sock, char* nfile, size_t filesize)
{
  int pfile, mlength = 0, receiveall, byte_ricevuti;
  
  pfile = open(nfile, O_WRONLY|O_CREAT|O_TRUNC, 0660);
  if(pfile == -1)
    return -1;
  
  char* pbuff = malloc(READ_BUFFER_SIZE);
  if(pbuff == (char*)-1)
    return -1;
  
  //Se non è specificata la dimensione del file imposta in modo che sia inviato tutto
  if(filesize == 0)
    receiveall = 1;
  else 
    receiveall = 0;   
  
  do
  {
    //memset(pbuff, 0x0, READ_BUFFER_SIZE);
    byte_ricevuti = receiveData(ds_sock, pbuff, READ_BUFFER_SIZE);
    if(byte_ricevuti == -1)
    {
      close(pfile);
      free(pbuff);
      return -1;
    }
    
    if(byte_ricevuti == 0)
      break;
    
    write(pfile, pbuff, byte_ricevuti);
    
    mlength += byte_ricevuti;
  }
  while(mlength < filesize || receiveall);
  
  free(pbuff);
  
  return pfile;
}


/*Trasferisce il file aperto su pfile di dimensione fileSize sulla ds_sock, tornando la quantità di byte realmente trasferiti
 */
int trasferFile(int ds_sock, int pfile, size_t filesize)
{
  int byte_letti, readall, mlength = 0;
  char* tempbuff = malloc(READ_BUFFER_SIZE);
  
  //Se non viene specificata la dimensione del file, imposta in modo che sia inviato tutto il file
  if(filesize == 0)
    readall = 1;
  else 
    readall = 0;
  
  do
  {    
    byte_letti = read(pfile, tempbuff, READ_BUFFER_SIZE);
    if(byte_letti == -1)
    {
      if(errno == EINTR)
	continue;
      else
      {
	free(tempbuff);
	return -1;
      }
    }
    else if(byte_letti == 0)
      return mlength;
    
    mlength += byte_letti;
    
    if( sendData(ds_sock, tempbuff, byte_letti) == -1)
    {
      free(tempbuff);
      return -1;
    }
  }
  while(mlength < filesize || readall);
  
  free(tempbuff);
  
  return mlength;
}
