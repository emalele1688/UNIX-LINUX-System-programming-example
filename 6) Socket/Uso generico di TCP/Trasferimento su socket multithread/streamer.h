#ifndef __BOOK_STREAMER_ 
#define __BOOK_STREAMER_ 

#include <stddef.h>


#ifndef SSIZE_MAX
#define SSIZE_MAX	sizeof(ssize_t)
#endif


size_t read_block(int file_descriptor, void *dest_buff, size_t file_size);


size_t write_block(int file_descriptor, const void *sorgbuff, size_t file_size);


#endif