#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>

void printmess()
{
  printf("Lanciata! con CTRL+c non si esce subito, poichè è bloccato fino alla fine di esecuzione di questo segnale\nPerò, poichè non è catturato si uscirà subito dopo\n");
  sleep(3);
}

void printmess2()
{
  printf("Lanciata! con CTRL+c si esce\n");
  sleep(3);
}

int main(int argc, char* args[])
{
  struct sigaction sigact; 
  struct sigaction sigact2;
  
  //sa_mask in sigaction è un sigset_t che va inizializzato. Conterrà i segnali da bloccare durante la gestione di un segnale qualsiasi. Un alternativa vedi 1 dopo il programma
  sigemptyset(&sigact.sa_mask);
  //Specifica dei comportamenti alla recezione del segnale (vedi documentazione kernel: http://www.kernel.org/doc/man-pages/online/pages/man2/sigaction.2.html)
  sigact.sa_flags = 0;
  //Funzione di gestione del segnale
  sigact.sa_handler = printmess;
  
  //Inseriamo un segnale nell'insieme che sarà bloccato fino alla fine della gestione di un altro segnale
  sigaddset(&sigact.sa_mask, SIGINT);
  //Associamo al segnale SIGUSR1 l'azione (sigaction) sigact (oldact è NULL poichè è la prima azione per tale segnale)
  sigaction(SIGUSR1, &sigact, NULL);
  
  //Imposto un SIGUSR2 che può essere gestito in qualsiasi momento
  sigemptyset(&sigact2.sa_mask);
  sigact2.sa_flags = 0;
  sigact2.sa_handler = printmess2;
  sigaction(SIGUSR2, &sigact2, NULL);
  
  //Mi lancio il segnale dasolo per comodita
  printf("Sto per lanciarmi una SIGUSR1! non si potrà uscire\n");
  raise(SIGUSR1);
  
  printf("Sto per lanciarmi una SIGUSR2! si potrà uscire\n");
  raise(SIGUSR2);
  
  //Verifichiamo la presenza di uno specifico segnale in un insieme (torna 1 se presente, 0 altrimenti)
  if(sigismember(&sigact.sa_mask, SIGINT))
    printf("SIGINT presente nell'insieme\n");
  
  if(!sigismember(&sigact.sa_mask, SIGTERM))
    printf("SIGTERM non presente nell'insieme\n");
  
  return 0;
}

/*1:
 * struct sigaction sigact;
 * sigset_t sigset;
 * sigemptyset(&sigset);
 * sigact.sa_mask = sigset;
 * ...
 */