#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include "concurrent_streamer.h"



#define BLOCK_BUFFER_SIZE	104857600
#define METADATA_SIZE sizeof(struct concurrent_streamer)


struct concurrent_streamer
{
  void* front_buffer;
  void* back_buffer;
  sem_t front_buffer_lock;
  sem_t back_buffer_lock;
  pthread_t t_reader;
  pthread_t t_writer;
  size_t bufferValid;
  int input_descriptor;
  int output_descriptor;
  size_t data_size;
  size_t data_transferred_size;
  int errsv;
};


void *cstreamer_read(void* _str);
void *cstreamer_write(void* _str);


int cstreamer_init(cstreamer_t** str)
{
  sem_t front_sem, back_sem;
  int errsv = 0;

  if((sem_init(&front_sem, 0, 0)) == -1)
  {
    errsv = errno;
    return errsv;
  }
  
  if((sem_init(&back_sem, 0, 1)) == -1)
  {
    errsv = errno;
    sem_destroy(&front_sem);
    return errsv;
  }
  
  void *mblock = malloc((BLOCK_BUFFER_SIZE * 2) + METADATA_SIZE);
  if(mblock == 0)
  {
    errsv = errno;
    sem_destroy(&front_sem);
    sem_destroy(&back_sem);
    return errsv;
  }
  memset(mblock, 0, (BLOCK_BUFFER_SIZE * 2) + METADATA_SIZE);
  
  *str = mblock;
  memcpy(&((*str)->front_buffer_lock), &front_sem, sizeof(front_sem));
  memcpy(&((*str)->back_buffer_lock), &back_sem, sizeof(back_sem));
  (*str)->front_buffer = ((char*)mblock) + METADATA_SIZE;
  (*str)->back_buffer = ((char*)mblock) + METADATA_SIZE + BLOCK_BUFFER_SIZE;

  return 0;
}

int cstreamer_start(cstreamer_t* str, int input_descriptor, int output_descriptor, size_t file_size)
{
  int ret;
  
  str->input_descriptor = input_descriptor;
  str->output_descriptor = output_descriptor;
  str->data_size = file_size;

  if((ret = pthread_create(&str->t_reader, NULL, cstreamer_read, str)) != 0)
   return errno;
  
  if((ret = pthread_create(&str->t_writer, NULL, cstreamer_write, str)) != 0)
   return errno;
  
  return 0;
}

int cstreamer_wait_transferer(cstreamer_t *str, size_t *byte_transferred)
{
  pthread_join(str->t_reader, NULL);
  pthread_join(str->t_writer, NULL);
  
  if(byte_transferred != 0)
    *byte_transferred = str->data_transferred_size;

  return str->errsv;
}

void cstreamer_destroy(cstreamer_t* str)
{
  sem_destroy(&(str->front_buffer_lock));
  sem_destroy(&(str->back_buffer_lock));
  free(str);
}


void *cstreamer_read(void* _str)
{
  cstreamer_t *str = (cstreamer_t*)_str;
  
  size_t byteCounter = str->data_size;
  size_t readerBlockSize = BLOCK_BUFFER_SIZE;
  size_t byteReaded = 0;
  
  while(byteCounter > 0)
  {
    if(byteCounter < BLOCK_BUFFER_SIZE)
      readerBlockSize = byteCounter;
    
    byteReaded = read(str->input_descriptor, str->front_buffer, readerBlockSize);
    if(byteReaded == -1)
    {
      if(errno == EINTR)
	continue;
      else
      {
	str->errsv = errno;
	break;
      }
    }
    else if(byteReaded == 0)
      break;

    sem_wait(&(str->back_buffer_lock));
    //Se il writer è terminato prima del previsto allora la reader termina
    if(str->bufferValid == -1)
    {
      str->data_transferred_size = (str->data_size - byteCounter);
      pthread_exit(0);
    }
    str->bufferValid = byteReaded;
    //SWAP
    void* temp = str->back_buffer;
    str->back_buffer = str->front_buffer;
    str->front_buffer = temp;
    //End swap => back buffer is ready
    sem_post(&(str->front_buffer_lock));
    
    byteCounter -= byteReaded;
  }
  
  sem_wait(&(str->back_buffer_lock));
  str->bufferValid = 0;
  sem_post(&(str->front_buffer_lock));
  
  str->data_transferred_size = (str->data_size - byteCounter);
  pthread_exit(0);
}


void *cstreamer_write(void* _str)
{
  cstreamer_t* str = (cstreamer_t*)_str;
  size_t btw;
  size_t ret;
  
  sem_wait(&(str->front_buffer_lock));
  while(str->bufferValid > 0)
  {
    btw = str->bufferValid;
    while(btw > 0)
    {
      ret = write(str->output_descriptor, str->back_buffer, btw);
      if(ret == -1)
      {
	if(errno == EINTR)
	  continue;
	else
	{
	  str->errsv = errno;
	  str->bufferValid = -1;
	  sem_post(&(str->back_buffer_lock));
	  pthread_exit(0);
	}
      }
      btw -= ret;
    }

    sem_post(&(str->back_buffer_lock));    
    sem_wait(&(str->front_buffer_lock));
  }
  
  pthread_exit(0);
}