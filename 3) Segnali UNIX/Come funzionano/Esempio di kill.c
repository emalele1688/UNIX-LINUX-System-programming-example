#include <signal.h>
#include <sys/types.h>

int main(int arg, char* args[])
{
  pid_t process;
  printf("Inserisci l'id del processo da terminare\n");
  scanf("%d", &process);
  if( kill(process, SIGKILL) == -1)
    printf("Impossibile terminare il processo\n");
  
  // Analogamente la funzione raise(sig) invia un segnale al processo corrente
  
  return 0;
}

