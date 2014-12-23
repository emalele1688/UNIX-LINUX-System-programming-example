#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sched.h>
#include <time.h>
#include <string.h>
#include <errno.h>


#ifndef SCHED_BATCH
#define SCHED_BATCH	3
#endif
#ifndef SCHED_IDLE
#define SCHED_IDLE	5
#endif

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
  if(argn != 4)
  {
    printf("Usage: priority [policy: 1=OTHER, 2=BATCH, 3=IDLE] [nice] [Input size (es: 100)]\n");
    return 0;
  }
  
  int policy = 0;
  switch(atol(args[1]))
  { 
    case 1:
      policy = SCHED_OTHER;
      printf("Scheduler OTHER\n");
      break;
    
    case 2:
      policy = SCHED_BATCH;
      printf("Scheduler BATCH\n");
      break;
      
    case 3:
      policy = SCHED_IDLE;
      printf("Scheduler IDLE\n");
      break;
    
    default:
      policy = 0; //Normal (OTHER) scheduler
  }

  /* OTHER: Scheduler ordinario
   * BATCH: Scheduler ordinario ma non favorisce i processi che passano da sleep a execute
   * IDLE: Processi con bassissima priorità
   */
  struct sched_param prior;
  prior.sched_priority = 0;
  if( sched_setscheduler(0, policy, &prior) == -1)
  {
    printf("%s\n", strerror(errno));
    return -1;
  }
  
  int ret = nice(atol(args[2]));
  if(ret == -1)
  {
    printf("%s\n", strerror(errno));
    return -1;
  }
  printf("Livello di priorità impostato: %d\n", ret);
  
  unsigned long int arraysize = MOLTIPLICATOR * atol(args[3]);
  printf("%ld elementi interi nell'array\n", arraysize);
  int* vect = malloc(sizeof(int)*arraysize);
  
  srand(time(NULL));
  
  unsigned long int i = 0;
  for(i = 0; i < arraysize; i++)
    vect[i] = rand() % 1000;
  
  sort(vect, arraysize);

  return 0;
} 
