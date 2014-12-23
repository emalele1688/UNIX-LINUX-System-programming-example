#include <stdio.h>
#include <math.h>

/* Il comando RDTSC legge il tempo dal Time Stamp Counter, che è un registro da 64 bit che si incrementa di uno ad ogni colpo di clock del TSC 
 * Il TSC è calibrato al boot dal kernel con la frequenza del processore
 */

int frequency_of_primes (int n) 
{
  int i,j;
  int freq=n-1;
  for(i=2; i<=n; ++i) 
  {
    for(j=sqrt(i); j>1; --j) 
      if(i%j ==0) 
      {
	--freq; 
	break;
      }
  }
  return freq;
}

void access_counter(unsigned int* hi, unsigned int* lo)
{
  /* rdtsc: Non ha input - torna in edx 32 bit più significativi, eax 32 bit meno significativi
   */
  asm(
    "rdtsc" :
    "=a" (*lo), "=d" (*hi)
  );
  
  /* Cenno sulla sintassi:
   * "rdtsc" => Istruzione
   * "=a (*lo)" Contenuto di eax in *lo
   * "=d (*hi)" Contenuto di edx in *hi
   */
}

int main()
{
  int sthi, stlo, enhi, enlo;
  printf("Attendere...\n");
  //Accedi al contatore
  access_counter(&sthi, &stlo);
  int fp = frequency_of_primes(2000000);
  printf("Numeri primi in 2000000 = %d\n", fp);
  access_counter(&enhi, &enlo);
  printf("Cicli di clock del TSC impiegati: %ld.%ld \n", enhi-sthi, enlo - stlo); 
  
  return 0;
}
