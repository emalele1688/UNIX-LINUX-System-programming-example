/*Vengono definite le funzioni di trasferimento file 
 * Tra le memorie degli host e tra file su disco degli host
 */

#ifndef TRASFER_MAN_H
#define TRASFER_MAN_H



#include <unistd.h>


#define 	READ_BUFFER_SIZE	4096 


/*Leggi max_buffer_length (o di meno) byte dalla socket ds_sock e memorizzali in buffer
 */
int receiveData(int ds_sock, void* buffer, size_t max_buffer_length);


/*Invia sulla ds_sock il messaggio message di lunghezza lengthToWrite.
 * Se il messaggio è più lungo di lengthToWrite sarà inviata solo la porzione lengthToWrite
 */
int sendData(int ds_sock, const void* message, size_t lengthToWrite);


/*Riceve un file dalla ds_sock di dimensione massima filesize salvandolo in un file di nome nfile sul disco tornandone il descrittore
 */
int receiveFile(int ds_sock, char* nfile, size_t filesize);


/*Trasferisce il file aperto su pfile di dimensione fileSize sulla ds_sock, tornando la quantità di byte realmente trasferiti
 */
int trasferFile(int ds_sock, int pfile, size_t filesize);



#endif
