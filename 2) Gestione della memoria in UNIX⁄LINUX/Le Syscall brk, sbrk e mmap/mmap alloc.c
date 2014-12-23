#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/mman.h>

#define SIZE 1000000000

int main(void)
{
  int i;
  
  //Confronto indirizzi con malloc
  void* pm1 = malloc(1024);
  printf("Malloc a indirizio %u\n", pm1);
  
  void* p = mmap(0, SIZE * sizeof(int), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  
  printf("Reggione di memoria mappata all'indirizzo %u\n", p);
  printf("Reggione di memoria finale %u\n", (int*)p+SIZE);
  
  printf("Tra malloc e mmap distano %u byte\n", (unsigned long int)p - (unsigned long int) pm1);
  
  if(p == MAP_FAILED)
  {
    printf("%s\n", strerror(errno));
    exit(1);
  }
  
  memset(p, 0, SIZE * sizeof(int));
  
  for(i = 0; i < SIZE; i++)
    *((int*)p) = i;
  
  double vc = 0;
  for(i = 0; i < SIZE; i++)
    vc += *((int*)p);
  vc /= SIZE;
  
  printf("%f\n", vc);
  
  munmap(p, SIZE * sizeof(int));
  free(pm1);
  
  return 0;
}
