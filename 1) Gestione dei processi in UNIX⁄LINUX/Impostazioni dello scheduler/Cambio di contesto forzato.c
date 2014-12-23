#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sched.h>
#include <string.h>
#include <errno.h>


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
    //Forzo il cambio di contesto
    if(sched_yield() == -1)
      printf("%s\n", strerror(errno));
  }
}


int main(int argn, char* args[])
{    
  const unsigned long int arraysize = 100000;
  int* vect = malloc(arraysize*sizeof(int));
  
  srand(time(NULL));
  
  unsigned long int i = 0;
  for(i = 0; i < arraysize; i++)
    vect[i] = rand() % 1000;
  
  //Imposto priorità FIFO al processo per dare un senso
  struct sched_param prior;
  prior.sched_priority = 99;
  if(sched_setscheduler(0, SCHED_FIFO, &prior))
    printf("%s\n", strerror(errno));
  
  sort(vect, arraysize);
    
  return 0;
}
