#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>


void sighup(int sig)
{
  FILE* fd = fopen("sighup", "w");
  fprintf(fd, "SIGHUP");
  fclose(fd);
}


void sigusr1(int sig)
{
  printf("Processo %d scrive: ricevuto segnale SIGUSR1\n", getpid());
}

void sigchld(int sig)
{
  printf("Un processo figlio è terminato\n");
}

void processo1()
{
  printf("Processo %d scrive:\n\tSono il nuovo processo del gruppo %d sessione %d\n", getpid(), getpgrp(), getsid(getpid()));
  int newgrp = setpgrp();
  printf("Processo %d scrive:\n\tHo creato un nuovo gruppo e ne sono il leader. Il suo pgid è: %d\n\tSono sempre sulla sessione %d\n", getpid(), getpgrp(), getsid(getpid()));
  sleep(5);
 
  //Ogni processo creato qui sotto apparterrà a questo gruppo (salvo creazione nuovo gruppo)
  //All'uscita sarà inviato un segnale SIGCHLD catturato
  exit(0);
}

void processo2()
{
  printf("Processo %d scrive:\n\tSono il nuovo processo del gruppo %d sessione %d\n", getpid(), getpgrp(), getsid(getpid()));
  sleep(5);
  //All'uscita sarà inviato un segnale SIGCHLD catturato
  exit(0);
}

void go()
{
  //PROCESS LEADER ROOT
  
  printf("Processo %d scrive:\n\tOra creo due processi che apparterranno alla mia stessa sessione e mio stesso gruppo \n", getpid());
  
  int p1, p2;
  
  //GRUPPO 1 di questa sessione
  p1 = fork();
  if(p1 == 0)
    processo1();
 
  //GRUPPO 2 di questa sessione
  p2 = fork();
  if(p2 == 0)
    processo2();
  
  printf("Processo %d scrive:\n\tAttendo i due gruppi che terminano\n", getpid());
  wait(0);
  wait(0);
  
  //Alla sua uscita il padre è terminato, quindi non riceverà il segnale SIGCHLD
  exit(0);
}

int main()
{
  /* REMEMBER:
   * Gruppo di processi: Servono per la consegna dei segnali (killpg) e la condivisione del terminale. Ogni process group, identificato dal suo pgid, condivide il terminale di sessione con gli altri process group della stessa sessione.
   * I segnali sono consegnati individualmente ad ogni gruppo di processi.
   * Ogni processo avviato dal sistema è leader di un gruppo. Tutti i processi avviati dal leader (con fork) appartengono al suo gruppo (hanno tutti lo stesso pgid).
   * Ogni processo che non è leader di un gruppo può creare un nuovo gruppo e diventare il suo leader.
   */
  /* Sessioni: Ogni sessione ha uno o più gruppi di processi associati che condividono il terminale di sessione. La sessione è identificata dal sid.
   * Un processo che appartiene ad una sessione 'sid 1' non può entrar a far parte di un gruppo di processi sulla sessione 'sid 2'. Può entrar a fa parte solo dei gruppi di processi presenti nella sua sessione.
   */
  
  
  //L'impostazione dei gestori di segnali sono validi per tutti i figli che sono creati a partire da questo processo, anche su sessioni differenti
  signal(SIGUSR1, &sigusr1);
  signal(SIGCHLD, &sigchld);
  //Il processo padre ha il terminale, termina subito, ed i processi figli anche se con sessione diversa vengono adottati da init, per cui la chiusura del terminale durante la loro esecuzione non invierà il segnale SIGHUP (vedi Provocare sighup.c)
  //Se però riassocio il terminale del padre ai figli con sessione diversa allora, il processo leader della sessione invierà una SIGHUP a tutti i suoi figli che appartengono alla stessa sessione
  signal(SIGHUP, &sighup);
  
  pid_t ret = 0;
  pid_t dad = getpid();
  printf("Processo %d scrive:\n\tLa sessione di partenza è: %d\n\tSono leader del gruppo %d\n", getpid(), getsid(getpid()), getpgrp());
  
  /* Il processo padre è leader del gruppo di tutti i processi che saranno creati da lui con fork.
   * Essendo lui un leader di gruppo non può staccarsi dalla sua sessione.
   * Creo quindi un suo figlio (che inizialmente apparterrà al suo gruppo ed alla sua sessione). Questo figlio, non essendo leader di un gruppo, può staccarsi dal padre e creare una nuova sessione con un nuovo gruppo dove lui diventerà il leader
   */
  ret = fork();
  
  if(ret == 0)
  {
    //Ritardo per far terminare il padre (dimostrativo)
    printf("Processo %d scrive:\n\tSalve, sono il processo figlio di %d\n\tLa mia sessione è sempre %d ereditata da mio padre\n\tAppartengo al gruppo %d\n", getpid(), dad, getsid(getpid()), getpgrp());
    
    /* Il processo figlio invoca setsid creando una nuova sessione, con un nuovo gruppo di processi dove lui diventa il leader
     * Il sistema provvede ad assegnare alla nuova sessione un terminale di controllo.
     * La nuova sessione contiene tutti i file descriptor aperti fin'ora, quindi anche quelli aperti dal padre.
     */
    if(setsid() == -1)
      printf("Processo %d scrive:\n\tImpossibile creare un leader per questa sessione: %s\n", getpid(), strerror(errno));   
    
    printf("Processo %d scrive:\n\tAvvio una nuova sessione\n\tLa mia nuova sessione è: %d\n\tSono il leader del nuovo gruppo %d\n", getpid(), getsid(getpid()), getpgrp());

    go();
    
  }
  
  printf("Processo padre %d scrive: Esco\n", getpid());
  
  //Il processo padre termina l'esecuzione, il figlio continua ma non ha il terminale associato
  return 0;
}
