#include <stdio.h>
#include <unistd.h>

int *myMalloc(int size)
{
  //sbrk mi ritorna un puntatore per size byte, dedicati al mio processo nell'heap. I size byte non sono allocati, ma pronti per essere usati. (ret = (inizio del program break)). Dopo, sbrk(0) = ret + size
  int *ret = sbrk(size);
  //Tutte le celle (sono da 1 byte ciascuna) inizializzato a 11111111 = 255
  memset(ret, 0xFF, size);
  //Ritorna l'inizio del puntatore
  return ret;
}

int main(int argc, char *args)
{
  //sbrk punta all'inizio del program break del mio processo. Con 0 ritorna l'inizio della parte non allocata (program_break_start + 0)
  printf("Program break iniziale del mio processo ( sbrk(0) ) %u\n", sbrk(0));
  int size;
  printf("Quanta memoria vuoi allocare? (byte)\n");
  scanf("%d", &size);
  //Alloca size byte - Se inizio a scrivere da sbrk(0) HO UNA segmentation fault, mi riservo size byte con sbrk(size)
  int *p_start = myMalloc(size);
  //p_start; punta all'inizio dello spazio allocato - sbrk(0) ora ritorna la fine, ovvero l'inizio del Program Break non allocato
  printf("Memoria allocata da %u ", p_start);
  printf(" a %u\n", sbrk(0));
  
  printf("La carico\n");
  int i, *curr = p_start;
  //size / sizeof(int) un int son 4 byte --- Se alloco 32 byte avrò esattamente 8 locazioni disponibili per ogni numero int 
  for(i = 0; i < size/sizeof(int); i++)
  {
    *curr = i;
    curr += 1;
  }
  
  curr = p_start;
  for(i = 0; i < size/sizeof(int); i++)
  {
    printf("A posizione %u = %d\n", curr, *curr);
    curr += 1;
  }
  
  //Program break non inizializzato
  printf("Il program brak non allocato per questo processo parte da ( sbrk(0) ) %u\n", sbrk(0));
  
  return 0;
}
