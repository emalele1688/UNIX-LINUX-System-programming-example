#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <signal.h>

#define NSEDIE 5
#define NCLIENTI 10

sem_t *sem_b; //Semaforo che indica il numero di clienti in attesa. Se 0 addormenta il barbiere
sem_t *sem_c; //Se il barbiere serve un cliente metti il nuovo cliente in attesa

void stop()
{
  printf("Dealloco i semafori\n");
  sem_close(sem_b);
  sem_close(sem_c);
  sem_unlink("barbiere");
  sem_unlink("cliente");
  
  exit(0);
}

void cliente(int id)
{
  //Controllo che ci siano abbastanza sedie
  int nclienti;
  sem_getvalue(sem_b, &nclienti);  
  if(nclienti == NSEDIE-1)
  {
    printf("Non ho abbastanza sedie per il cliente %d\n", id);
    exit(1);
  }
  
  //Sveglio/Avverto il barbiere della mia presenza incrementando il numero di clienti in attesa
  sem_post(sem_b);
  //Attendo che mi vengano tagliati i capelli
  printf("%d si siede\n", id);
  sem_wait(sem_c);
  printf("I capelli di %d sono stati tagliati\n", id);
  
  exit(0);
}

void barbiere()
{
  while(1)
  {
    int nclienti;
    sem_getvalue(sem_b, &nclienti);
    printf("Ho %d clienti da servire\n", nclienti);
    
    //Attendo clienti, se ho clienti da servire questo valore è uguale al numero di clienti da servire
    sem_wait(sem_b);
    printf("Taglio i capelli a:\n");
    //Simulazione del taglio
    sleep(1);
    sem_post(sem_c);
  }
  
  exit(0);
}

int main(int arg, char* args[])
{  
  signal(SIGINT, stop);

  sem_b = sem_open("barbiere", O_CREAT|O_EXCL, 0666, 0);
  sem_c = sem_open("cliente", O_CREAT|O_EXCL, 0666, 0);
  
  int i, ret;
  
  //crea barbiere
  ret = fork();
  if(ret == 0)
    barbiere();
  
  //crea clienti  
  for(i = 0; i < NCLIENTI; i++)
  {
    ret = fork();
    if(ret == 0)
      cliente(i);
  }
  
  for(i = 0; i < NCLIENTI+1; i++)
    wait();
  
  return 0;
}
  
