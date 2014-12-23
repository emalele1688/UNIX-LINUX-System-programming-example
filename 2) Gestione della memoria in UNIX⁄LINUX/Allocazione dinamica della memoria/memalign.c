#include <malloc.h>
#include <string.h>
#include <errno.h>

int main(int argn, char* args[])
{
  //Alloca memoria alineata di page size
  void* pbuf = memalign(32, 64);
  printf("Il blocco ha dimensione %lu\n", *((long*)pbuf - 1));
  printf("La memoria allocata inizia a %lu\n", pbuf);
  free(pbuf);
  
  //pointer, alignment, size
  if(posix_memalign(&pbuf, 32, 128) != 0)
  {
    printf("Errore in posix_memalign: %s\n", strerror(errno));
    return 1;
  }
  printf("Il blocco ha dimensione %lu\n", *((long*)pbuf - 1));
  printf("La memoria allocata inizia a %lu\n", pbuf);
  free(pbuf);
  
  return 0;
}
