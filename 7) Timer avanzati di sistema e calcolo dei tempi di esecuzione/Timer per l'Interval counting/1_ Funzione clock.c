#include <stdio.h>
#include <math.h>
#include <time.h>
#include <sys/resource.h>

//Dato n, calcola da 0 a n quanti numeri primi sono presenti - Serve per perdere tempo per la dimostrazione
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

int main(void)
{ 
  clock_t clk;
  
  printf("La funzione clock è calibrata a %lu cicli al secondo\n", CLOCKS_PER_SEC);
  
  //Fai qualcosa
  int f = frequency_of_primes(2000000);
  printf ("La frequenza dei numeri primi di 2000000 è: %d\n",f);  
  
  clk = clock();
  printf("Tempo trascorso dall'avvio del processo %f\n", ((float)clk / CLOCKS_PER_SEC));
  
  printf("Il contatore clock calcola il tempo con la tecnica del interval counting, per dimostrarlo entro in sleep\n");
  sleep(3);
  clk = clock();
  printf("Il tempo trascorso dall'avvio del processo è rimasto %f\n", ((float)clk / CLOCKS_PER_SEC));
  return 0;
}
