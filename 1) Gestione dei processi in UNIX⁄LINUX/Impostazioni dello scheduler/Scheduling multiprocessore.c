#include <sched.h>
#include <stdio.h>
#include <unistd.h>
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
  }
}

int main(int argn, char* args[])
{
  cpu_set_t new_cpu_mask;
  cpu_set_t restore_cpu_mask;
  
  //Inizializzo la cpu mask
  CPU_ZERO(&new_cpu_mask);
  //Inserisco il processore zero nella mask
  CPU_SET(0, &new_cpu_mask);
  
  //Prelevo ma maschera attualmente in uso (solo dimostrativo)
  if(shed_getaffinity(0, sizeof(restore_cpu_mask), &restore_cpu_mask) == -1)
    printf("%s\n", strerror(errno));
  
  /* Imposto l'affinità sul processore zero (il processo sarà sempre eseguito sul core zero)
   * shed_setaffinity(pid (0 per il processo corrente), sizeof mask, mask)
   */
  if(shed_setaffinity(0, sizeof(new_cpu_mask), &new_cpu_mask) == -1)
  {
     printf("%s\n", strerror(errno));
     return -1;
  }

  const unsigned long int arraysize = 100000;
  int* vect = malloc(arraysize * sizeof(int));

  srand(time(NULL));

  unsigned long int i = 0;
  for(i = 0; i < arraysize; i++)
    vect[i] = rand() % 1000;

    printf("%s\n", strerror(errno));
  
  sort(vect, arraysize);
  
  //Ripristino la maschera precedente (solo dimostrativo poichè il processo sta per terminare)
  if(shed_setaffinity(0, sizeof(restore_cpu_mask), &restore_cpu_mask) == -1)
    printf("%s\n", strerror(errno));
  
  return 0;
}