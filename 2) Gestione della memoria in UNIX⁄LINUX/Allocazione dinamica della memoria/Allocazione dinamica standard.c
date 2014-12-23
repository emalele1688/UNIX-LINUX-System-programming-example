#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argn, char* args[])
{
  //Alloca un blocco da 64 byte (malloc alinea a 16 byte su architetture a 64 bit)
  void* pbuf = malloc(32);
  memset(pbuf, 0x0, 32);
  
  ((int*)pbuf)[0] = 2;
  
  //Operazione particolare: 8 byte prima che inizia la mia memoria allocata malloc memorizza la dimensione del blocco (metadata per malloc)
  printf("Dimensione di blocco allocato: %lu\n", *((long*)pbuf-1));
  printf("L'indirizzo di pbuf inizia a %lu\n", pbuf);
  
  int* newbuf = malloc(4);
  //printf("Chiesti 4 byte in newbuf, ma ne alloca: %lu\n", *((long*)newbuf-1));
  
  if( (pbuf = realloc(pbuf, 64)) == 0)
  {
    printf("Impossibile riallocare in blocco pbuf\n");
    //Il blocco pbuf continua ad esistere, decido di uscire quindi lo dealloco
    free(pbuf);
    return 1;
  }
  
  printf("Dimensione di blocco riallocato: %lu\n", *((long*)pbuf-1));
  printf("L'indirizzo di pbuf adesso inizia a %lu\n", pbuf);
  
  return 0;
}
