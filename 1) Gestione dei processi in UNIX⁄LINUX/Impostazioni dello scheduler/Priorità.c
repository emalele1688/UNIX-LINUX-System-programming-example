#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#define MOLTIPLICATOR 1000


void sort(int v[], unsigned long int dim)
{
  unsigned long int i = 0, j = 0;
    
  for(i = 0; i < dim-1; i++)
  {
    for(j = i+1; j < dim; j++)
    {
      if(v[i] < v[j])
      {
	int temp = v[i];
	v[i] = v[j];
	v[j] = temp;
      }
    }
  }
}


int main(int argn, char* args[])
{
  if(argn != 3)
  {
    printf("Usage: priority [niceness=(-20,19)] [input_size=(0,1000)]\n");
    return 0;
  }
  
  /* Imposto la priorità al processo 
   * Il valore nice è inteso come un livello di cortesia di questo processo nei confronti degli altri
   * Più sarà alto meno sarà la priorità del processo
   * Torna il livello nice impostato, -1 in caso di errore
   */
  int ret = nice(atol(args[1]));
  if(ret == -1)
  {
    printf("%s\n", strerror(errno));
    return -1;
  }
  printf("Livello di priorità impostato: %d\n", ret);
  
  unsigned long int arraysize = MOLTIPLICATOR * atol(args[2]);
  printf("%d elementi interi nell'array\n", arraysize);
  int* vect = malloc(sizeof(int)*arraysize);
  
  srand(time(NULL));
  
  unsigned long int i = 0;
  for(i = 0; i < arraysize; i++)
    vect[i] = rand() % 1000;
  
  sort(vect, arraysize);
    
  return 0;
}
