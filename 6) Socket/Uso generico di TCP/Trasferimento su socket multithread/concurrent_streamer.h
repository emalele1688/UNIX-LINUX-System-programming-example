#ifndef TRANSFERER_H
#define TRANSFERER_H


#include <stddef.h>


struct concurrent_streamer;

typedef struct concurrent_streamer cstreamer_t;

int cstreamer_init(cstreamer_t** str);

int cstreamer_start(cstreamer_t* str, int input_descriptor, int output_descriptor, size_t file_size);

int cstreamer_wait_transferer(cstreamer_t *str, size_t *byte_transferred);

void cstreamer_destroy(cstreamer_t* str);


#endif