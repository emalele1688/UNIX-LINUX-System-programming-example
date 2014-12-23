#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/wait.h>

int main(int argc, char **argv) 
{
    char comand[256];
    unsigned int processid, retcomand;
    
    while(1)
    {
      printf("Digita un comando. Quit per uscire\n");
      scanf("%s", comand);
      
      if(strcmp(comand, "quit") == 0)
	 return 0;
      else
      {
	processid = fork();
	
	if(processid == -1)
	{
	  printf("Errore nel creare un nuovo processo\n");
	  return 1;
	}
	else if(processid == 0)
	{
	  /* execlp parameter: comand, arg[0], arg[1] ...
	   * esegue l'applicazione passata come path
	   * In questo caso viene passata la shell di comando, e come parametro iniziale la riga da eseguire (es. ls -l)
	   */
	  execlp(comand, comand, 0); //Sostituisce il processo figlio con comand
	  printf("%s\n", strerror(errno));
	  return -1; //se l'avvio di ececlp fallisce il processo figlio termina (esempio digito comand sbagliato)
	}
	else
	  waitpid(processid, &retcomand, 0);
	
	printf("Terminato con codice %d\n", retcomand);

	if(WIFEXITED(retcomand))
	  printf("Child uscito normalmente\n");
	if(WIFSIGNALED(retcomand))
	  printf("Child terminato a causa di un segnale non gestito\n");
	if(WIFSIGNALED(retcomand))
	  printf("Child terminato a causa del segnale %d non gestito\n", WTERMSIG(retcomand));
      }
    }
    
    return 0;
}
