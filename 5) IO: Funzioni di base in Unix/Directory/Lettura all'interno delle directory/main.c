#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>


int main(int arg, char* args[])
{
  DIR* dp;
  struct dirent *ep;
  
  //opendir apre una directory -- Restituisce il puntatore alla struttura DIR allocata dinamicamente
  if(arg == 1)
    dp = opendir("./");
  else
    dp = opendir(args[1]);
  
  if(dp == NULL)
  {
    printf("Impossibile aprire la directory corrente\n");
    exit(1);
  }

  printf("numero inode\t\tnome\t\ttipo file\t\tlenght\t\toffset prossimo file\n");
  /* readdir legge la prossima Dirent nella struct DIR che contiene un iteratore di Dirent, restituisce il puntatore alla Dirent attuale dell'iteratore
   * quando ep è 0 sono state iterate tutte le strutture a Dirent in DIR.
   * Si può invocare rewinddir per riportare l'iteratore delle Dirent alla prima Dirent
   */
  while( (ep = readdir(dp)) != 0)
  {
    printf("%d\t\t", ep->d_ino);
    printf("%s\t\t", ep->d_name);
    printf("%d\t\t", ep->d_type);
    printf("%d\t\t", ep->d_reclen);
    printf("%u\n", ep->d_off);
  }
  
  //Resetta la posizione della directory dp all'inizio NON SERVE QUI
  rewinddir(dp);

  if(closedir(dp) == -1)
    printf("Problemi nel chiudere il puntatore a directory\n");
  
  return 0;
}
