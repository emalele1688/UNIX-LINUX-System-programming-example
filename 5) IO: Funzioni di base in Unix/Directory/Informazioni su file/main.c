#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>

int main(int argn, char* args[])
{
  if(argn == 1)
  {
    printf("info [file1] [file2] ...\n");
    exit(0);
  }
  
  int i;
  //stat contiene informazioni per file/directory
  struct stat infof;
  for(i = 1; i < argn; i++)
  {
    //restituisce la stat associata al file o directory args
    if( stat(args[i], &infof) < -1)
    {
      printf("Impossibile sapere informazioni sul tipo di file specificato. Esiste?\n");
      exit(0);
    }
    
    printf("Numero inode: %d\n", infof.st_ino);
    printf("Device mount: %d\n", infof.st_dev);
    printf("Grandezza del file : %d bytes\n", infof.st_size);	
    printf("Numero link: %d\n", infof.st_nlink);
    printf("Grandezza blocchi nei trasferimenti su questo file %u\n", infof.st_blksize);
    printf("Numero blocchi allocati %u\n", infof.st_blocks);
    printf("Ultimo accesso %s", ctime(&infof.st_atime));
    printf("Ultima modifica %s", ctime(&infof.st_mtime));
    printf("Ultimo modifica ai dati dell'inode %s", ctime(&infof.st_ctime));
    
    printf("File Permissions: \t");
    /*Viene fatta l'and con ciascun tipo di permesso, se l'and è vero si stampa il permesso, altrimenti -
     * Ripasso dell'operatore ternario:
     * ( a > b ) ? c1 : c2
     * corrisponde a if(a>b) c1 else c2
     */
    printf( (S_ISDIR(infof.st_mode)) ? "d" : "-");
    printf( (infof.st_mode & S_IRUSR) ? "Il prorpietario ha permesso di lettura " : "-");
    printf( (infof.st_mode & S_IWUSR) ? "Il prorpietario ha permesso di scrittura " : "-");
    printf( (infof.st_mode & S_IXUSR) ? "Il prorpietario ha permesso di esecuzione " : "-");
    printf( (infof.st_mode & S_IRGRP) ? "Il gruppo ha permesso di lettura " : "-");
    printf( (infof.st_mode & S_IWGRP) ? "Il gruppo ha permesso di scrittura " : "-");
    printf( (infof.st_mode & S_IXGRP) ? "Il gruppo ha permesso di esecuzione " : "-");
    printf( (infof.st_mode & S_IROTH) ? "Gli altri hanno permesso di lettura " : "-");
    printf( (infof.st_mode & S_IWOTH) ? "Gli altri hanno permesso di scrittura " : "-");
    printf( (infof.st_mode & S_IXOTH) ? "Gli altri hanno permesso di esecuzione " : "-\n");
  }
  
  return 0;
}