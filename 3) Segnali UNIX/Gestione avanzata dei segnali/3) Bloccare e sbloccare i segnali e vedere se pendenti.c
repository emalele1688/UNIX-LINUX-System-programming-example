#include <signal.h>
#include <unistd.h>
#include <stdio.h>

void catcher( int sig ) 
{
  kill(getpid(), SIGUSR2);
  puts( "inside catcher(SIGUSR1) function\n" );
}

void catcher2( int sig ) 
{
  puts( "inside catcher2(SIGUSR2) function\n" );
}

void allarme(int sig) 
{
  puts( "ALLARME!\n" );
}

void check_pending( int sig, char *signame ) 
{
  sigset_t sigset;

  if(sigpending(&sigset) == -1)
      perror( "sigpending() error\n" );
  else if(sigismember(&sigset, sig) )
      printf( "a %s signal is pending\n", signame );
  else
      printf( "no %s signals are pending\n", signame );
}

int main( int argc, char *argv[] ) {

    struct sigaction sigact;
    struct sigaction sigact2;
    struct sigaction sigact3;
    sigset_t sigset;

    //SIGUSR1
    sigemptyset(&sigact.sa_mask);
    //Consegna dei segnali in ordine di invio per questo set
    sigaddset(&sigact.sa_mask, SIGUSR1);
    sigaddset(&sigact.sa_mask, SIGUSR2);
    sigact.sa_flags = 0;
    sigact.sa_handler = catcher;
    
    //SIGUSR2 - In sigact2 non tengo segnali bloccati durante l'esecuzione dell'sa_handler
    sigemptyset(&sigact2.sa_mask);
    sigact2.sa_flags = 0;
    sigact2.sa_handler = catcher2;

    sigaction(SIGUSR1, &sigact, NULL);
    sigaction(SIGUSR2, &sigact2, NULL);

    //Il segnale SIGUSR1 è momentaneamente bloccato
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);
    //Blocca i segnali nel set: SIG_SETMASK il set è quello specificato in sigset
    sigprocmask( SIG_SETMASK, &sigset, NULL );

    printf("SIGUSR1 è stato bloccato\n");
    kill(getpid(), SIGUSR1);
    printf( "after kill(SIGUSR1)\n" );

    check_pending( SIGUSR1, "SIGUSR1" );

    //Ma SIGUSR2 si invia
    printf("SIGUSR2 non è bloccato, infatti:\n");
    kill(getpid(), SIGUSR2);
    check_pending(SIGUSR2, "SIGUSR2"); //Non è pendente chiaramente
    
    //Sblocchiamo i segnali che sono stati bloccati. Se pendenti saranno serviti (nel nostro caso SIGUSR1)
    printf("Sblocco i segnali pendenti\n");
    sigemptyset( &sigset );
    /* Blocca i segnali: 
     * SIG_SETMASK: Blocca i segnali nel set, SIG_BLOCK: Aggiungi ai segnali già bloccati quelli nel set, SIG_UNBLOCK: I segnali specificati nel set sono sbloccati
     * Il terzo parametro (old_mask) restituisce la maschera dei segnali bloccati usata al momento della chiamata
     */
    sigprocmask( SIG_SETMASK, &sigset, NULL );
    printf("SIGUSR1 è stato sbloccato\n");
    check_pending(SIGUSR1, "SIGUSR1"); //Non più pendente
    
    printf("\nImposto un segnale di SIGALRM e lo blocco\n");
    
    //sigaction per SIGALRM - Non voglio consegne in ordine per SIGALRM (niente addset) solo sigaction per impostare il gestore
    sigemptyset(&sigact3.sa_mask);
    sigact3.sa_flags = 0;
    sigact3.sa_handler = allarme;
    sigaction(SIGALRM, &sigact3, NULL);

    //Blocco il SIGALRM
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGALRM);
    sigprocmask(SIG_SETMASK, &sigset, NULL);
    
    //Invio SIGALRM
    alarm(1);
    //Non esce dalla sleep poichè SIGALRM è bloccato
    char a[10];
    scanf("%s",a);
    
    printf("Dopo SIGALRM\n");
    check_pending(SIGALRM, "SIGALRM");
    
    //Sblocco il segnale SIGALRM pendente
    sigemptyset(&sigset);
    sigprocmask(SIG_SETMASK, &sigset, NULL );
    
    return 0;
} 
