#include <malloc.h>

int main(int argn, char* args[])
{
  //Alloca memoria alineata di page size
  void* pbuf = valloc(64);
  printf("Il blocco ha dimensione %lu\n", *((long*)pbuf - 1));
  printf("La memoria allocata inizia a %lu\n", pbuf);
  
  //Malloc non va in corruzione
  void* p = malloc(16);
  printf("Malloc inizia ad allocare a %lu\n", p);
  return 0;
}
