/* WARNING: Compatibile solo su architectura X86_64
 * testato con Intel I7 core 2600k
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>		//For ptrace
#include <sys/syscall.h>	//For SYS_write etc
#include <sys/user.h>		//For register access (in realtà non serve includerla)
#include <sys/reg.h>		//define ORIG_RAX (15) for X86_64 architecture


int main(int argn, char *args[])
{
  long orig_rax, rax;
  long params[3];
  pid_t child;
  int status = 0, insyscall = 0;
  
  if(argn < 2)
  {
    printf("tracer [executable]\n");
    exit(0);
  }
  
  //Creo il processo figlio che avvia l'esecuzione del processo da tracciare
  child = fork();
  if(child == -1)
  {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(-1);
  }
  if(child == 0) //Figlio
  {
    /* Il kernel traccierà questo processo ed i risultati saranno accessibili dal processo padre
     * PTRACE_TRACEME => Alla prima system call (quindi exec) il kernel invia una SIGTRAP al processo tracciato bloccandolo, ed una SIGCHLD al processo padre (il tracing process) che uscirà da un eventuale wait
     */
    if( ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1 )
    {
      fprintf(stderr, "%s\n", strerror(errno));
      exit(-1);
    }
    //Avvio il processo args[1]
    if( execl(args[1], args[1], NULL) == -1 )
    {
      fprintf(stderr, "%s\n", strerror(errno));
      exit(-1);
    }
    //Da qui in poi il processo figlio è sostituito dal codice di args[1]
  }
  else //Se sono il processo padre, eseguo il tracing!
  {
    while(1)
    {
      /* Ad ogni entrata ed uscita di una syscall (PTRACE_SYSCALL) nel processo tracciato, il Kernel invia al tracing procces (ovvero il padre) una SIGCHLD; (Cosi esce da wait)
       * Il processo tracciato invece viene bloccato inviando una SIGTRAP
       * Il valore di ritorno della wait è sempre il pid del processo atteso (il traced process nel nostro caso), ad eccezione per gli errori il quale torna -1
      */
      if(wait(&status) == -1)
      {
	if(errno == EINTR)
	  continue;
	else
	{
	  fprintf(stderr, "%s\n", strerror(errno));
	  exit(-1);
	}
      }

      //WIFEXITED Torna 1 se il processo è uscito normalmente => Quando il processo args[1] terminerà il tracer (processo padre) termina l'esecuzione
      if(WIFEXITED(status))
      {
	printf("exit\n");
	break;
      }
      
      if(WIFSTOPPED(status))
	printf("Process stopped by a signal number %d\n", WSTOPSIG(status));
      
      /* Quando il processo figlio chiama una system call il kernel salva il valore del registro rax (che appunto contiene il codice della systemcall) in una struttura di tipo user_regs_struct (definita in sys/user.h) al 15-esimo blocco
       * PTRACE_PEEKUSER restituisce un valore contenuto in tale struttura (che contiene tutti i valori dei registri del processo tracciato durante l'esecuzione)
       * Il valore da restituire va specificato nel campo 'address' di ptrace, ed in questo caso è: 8 * ORIG_RAX => 15-esimo campo della struttura user_regs_struct, ciascun campo è lungo 8 byte. (ORIG_RAX = 15 nelle architecture X86_64, 11 in X86)
       */
      orig_rax = ptrace(PTRACE_PEEKUSER, child, 8 * ORIG_RAX, NULL); 
      if(orig_rax == -1)
      {
	fprintf(stderr, "%s\n", strerror(errno));
	kill(child, SIGKILL);
	exit(-1);
      }
      
      /* Se è stata chiamata una write:
       * l'esecuzione della syscall è tracciata su due cicli: 
       * primo ciclo: sto per entrare nalla syscall; insyscall = 0
       * eseguo PTRACE_SYSCALL per continuare l'esecuzione del processo tracciato fino alla fine della syscall, quindi avvio un secondo ciclo dove insyscall = 1
       * alla fine del secondo ciclo insyscall si rimposta a zero e l'esecuzione del processo riparte, fino alla prossima syscall
       */
      if(orig_rax == SYS_write)
      {
	//Se sto per entrare nella systemcall write stampo i parametri con cui write è stata chiamata
	if(insyscall == 0)
	{
	  insyscall = 1;
	  
	  /* System call arguments (X86_64 calling convention) are put into registers %rdi, %rsi, %rdx, %r10, %r8 and %r9. in that order.
	   * Preleviamo i valori dei 3 parametri della syscall write
	   */	  
	  params[0] = ptrace(PTRACE_PEEKUSER, child, 8 * RDI, NULL);	//file descriptor
	  params[1] = ptrace(PTRACE_PEEKUSER, child, 8 * RSI, NULL);	//data pointer
	  params[2] = ptrace(PTRACE_PEEKUSER, child, 8 * RDX, NULL);	//length
	  
	  printf("write(%ld, %lx, %ld)\n", params[0], params[1], params[2]);
	}
	else //Se sono appena uscito dalla systemcall stampo il valore tornato dalla systemcall write
	{
	  rax = ptrace(PTRACE_PEEKUSER, child, 8 * RAX, NULL);
	  printf("Write returned %ld\n", rax);
	  
	  insyscall = 0;
	}
      }
      //Riprendi l'esecuzione del processo figlio fino alla prossima syscall, oppure fino alla fine della syscall che sta per entrare in esecuzione
      ptrace(PTRACE_SYSCALL, child, 0, 0);
    }
  }
  
  return 0;
}

/* Per leggere i valori dei registri sul primo ciclo potevamo anche usare
 * 	ptrace(PTRACE_GETREGS, child, NULL, &regs);
 * che preleva tutti i valori dei registri e li salva nella struttura del tipo user_regs_struct
 */