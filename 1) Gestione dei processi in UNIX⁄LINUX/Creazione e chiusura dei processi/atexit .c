#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

void foo1(void)
{
  printf("Chiusura foo1\n");
}

void foo2(void)
{
  printf("Chiusura foo2\n");
}

int main()
{
  //Registro delle funzioni da invocare in uscita
  atexit(foo1);
  atexit(foo2);
  printf("Uscita con codice 0\n");
  //Implementata nella libc internamente invoca _exit dopo aver invocato tutte le funzioni registrate da atexit
  exit(0);
  
  printf("Non sarà mai stampata questa stringa\n");
  return 0;
};
