
#ifndef __BOOK_STREAMER_ 
#define __BOOK_STREAMER_ 

#include <unistd.h>


#ifndef SSIZE_MAX
#define SSIZE_MAX	sizeof(ssize_t)
#endif


/* Legge dal file ds length byte mettondoli in destBuff
 * Torna -1 in caso di errore - Torna il numero di byte letti
 */
size_t readToStream(int ds, void* destBuff, size_t length);


/* Scrive sul file ds length byte del buffer sorgBuff
 * Torna -1 in caso di errore - Torna il numero di byte scritti
 */
size_t writeToStream(int ds, const void* sorgBuff, size_t length);

#endif