#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define SIZE_MAX 1024*1000000

void printmess2(int sig)
{
  //psignal stampa il nome del segnale
  psignal(sig, "Tipo segnale");
  printf("Gestore SIGUSR2!\n\n");
}

void printmess(int sig, siginfo_t* sinfo, void* context)
{
  //Dimostro come blocca i segnali:
  //Prima conclude questa chiamata poi esegue la gestione per questa raise(SIGUSR2)
  raise(SIGUSR2);
  psignal(sig, "Tipo segnale");
  printf("Alcuni dati della struttura sinfo:\n");
  //printf("\tTipo di segnale: %d\n", sinfo->);
  printf("\tProccess ID: %d\n", sinfo->si_pid);
  printf("\tTempo usr consumato: %u\n", sinfo->si_utime);
  printf("\tTempo sys consumato: %u\n", sinfo->si_stime);
  printf("\tSignal value %d\n", sinfo->si_value);
}

void backgroung(int sig)
{
  psignal(sig, "Tipo segnale");
  printf("Il segnale SIGSTP è stato catturato\n\n");
}

void stop(int sig)
{
  psignal(sig, "Tipo segnale");
  printf("Il segnale SIGINT è stato catturato\n\n");
}

void allarm(int sig)
{
  psignal(sig, "Tipo segnale");
  printf("Dovrei stare dentro una system call\n\n");
}

void violation(int sig, siginfo_t* sinfo, void* context)
{
  psignal(sig, "Tipo segnale");
  printf("Il segnale SIGSEGV è stato catturato, ma devo perforza uscire\n");
  if(sinfo->si_code == SEGV_MAPERR)//Address not mapped to object.
    printf("Inviato dal kernel\n\n");
  exit(0);
}

int main(int argc, char* args[])
{
  sigset_t set;
  //Per ogni segnale ho bisogno di installare la dovuta azione
  struct sigaction sigact1;
  struct sigaction sigact2;
  struct sigaction sigact3;
  struct sigaction sigact4;
  struct sigaction sigact5;
  struct sigaction sigact6;
  
  //Inizializzo la sigset_t
  sigemptyset(&set);
  
  //Aggiungo i segnali da bloccare e gestire al set
  sigaddset(&set, SIGUSR1);
  sigaddset(&set, SIGUSR2);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGTSTP);
  sigaddset(&set, SIGSEGV);
  sigaddset(&set, SIGALRM);
  
  /* Gestori dei segnali SIGUSR1 e SIGUSR2 - Con questi dimostro come vengono bloccati i segnali ------------------------------------------*/
  
  //Segnali da bloccare
  sigact1.sa_mask = set;
  //Flag per opzioni speciali, SA_SIGINFO specifica che uso sa_sigaction al posti di sa_handler
  sigact1.sa_flags = SA_SIGINFO;
  //Il gestore è sa_sigaction che riceve tre parametri in input (guarda printmess)
  sigact1.sa_sigaction = printmess;
  //Gestore del segnale SIGUSR2
  sigact2.sa_mask = set;
  sigact2.sa_flags = 0;
  //Il gestore qui invece è sa_handler che riceve solo il segnale catturato
  sigact2.sa_handler = printmess2;
  
  /* Gestore dei segnali SIGINT e SIGTSTP - Con questi blocco i due segnali e decido io che azione intraprendere --------------------------*/
  
  //Gestore del segnale SIGINT
  sigact3.sa_mask = set;
  sigact3.sa_flags = 0;
  sigact3.sa_handler = stop;
  //Gestore del segnale SIGTSTP
  sigact4.sa_mask = set;
  sigact4.sa_flags = SA_RESTART; //Questo flag non permette l'uscita dalla system call al ritorno della gestione del segnale
  sigact4.sa_handler = backgroung;
  
  /* Gestore dei segnali SIGSEGV e SIGALRM - Con questi blocco i due segnali e decido io che azione intraprendere --------------------------*/
  sigact5.sa_mask = set;
  sigact5.sa_flags = SA_SIGINFO;
  sigact5.sa_sigaction = violation;
  
  //Associamo al segnale SIGUSR1 l'azione sigact1
  sigaction(SIGUSR1, &sigact1, NULL);
  //Associamo al segnale SIGUSR2 l'azione sigact2
  sigaction(SIGUSR2, &sigact2, NULL);
  //Associamo al segnale SIGINT l'azione sigact3
  sigaction(SIGINT, &sigact3, NULL);
  //Associamo al segnale SIGTSTP l'azione sigact4
  sigaction(SIGTSTP, &sigact4, NULL);
  //Associamo al segnale SIGTSTP l'azione sigact5
  sigaction(SIGSEGV, &sigact5, NULL);
  
  //Mi lancio il segnale dasolo per comodita (siccome sono figo uso raise invece che kill(getpid(), SIGUSR1), non chè l'equivalente)
  printf("Sto per lanciarmi una SIGUSR1!\n");
  raise(SIGUSR1);
  printf("Dopo SIGUSR1\n");
  
  char scelta;
  printf("Vuoi testare una slow time sys call? (s/n) (scanf bloccante: se premi CTRL+z non esce - Con CTRL+c esce DALLA CHIAMATA (può rimanere bloccato nel buffer di input) )\n");
  scanf("%c", &scelta);
  while(getchar() != '\n');
  if(scelta == 's')
  {
    printf("La simulo con una write\n");
    int ds = open("temp", O_CREAT|O_RDWR);
    char* aa = malloc(SIZE_MAX);
    if(aa == 0)
      exit(0);
    memset(aa, 'a', SIZE_MAX);
    
    printf("Entro in system call\n");
    write(ds, aa, SIZE_MAX);
    printf("Esco in system call %s\n", strerror(errno));
    
    free(aa);
    unlink("temp");
    close(ds);
  }
  
  printf("Vuoi provocare una segmentation fault? (s/n) (scanf bloccante: se premi CTRL+z non esco - Con CTRL+c esce DALLA CHIAMATA)\n");
  scanf("%c", &scelta);
  if(scelta == 's')
  {
    void* p = (void*)50000;
    *((int*)p) = (int)4;
  }
   
  printf("Bye Bye!\n");
  return 0;
}
