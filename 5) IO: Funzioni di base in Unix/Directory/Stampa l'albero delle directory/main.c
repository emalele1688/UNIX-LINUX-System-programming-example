/*
  Esplora l'albero delle directory partendo dalla directory passata in input
    Dirent contiene informazioni su file/directory:
      struct dirent { 
	ino_t          d_ino;       /* inode number 
	off_t          d_off;       /* offset to the next dirent 
	unsigned short d_reclen;    /* length of this record 
	unsigned char  d_type;      /* type of file; not supported by all file system types 
	char           d_name[256]; /* filename 
    };
  scandir: prende un nome di una directory come input (dirp), invocando la funzione locale filter per ogni sottodirectory trovata in dirp passandogli la relativa dirent.
	    le dirent per cui filter non restituisce 0 sono allocate dinamicamente e inserite in un array allocato dinamicamente con l'ordine dato dalla funzione di sort passata
	    il puntatore all'array di Dirent è restituito per indirizzo (actdir)
	    Torna la grandezza dell'array di Dirent dinamico allocato
	    Attenzione: actdir è un puntatore ad array allocato con malloc che punta a un array di puntatori di Dirent allocati con malloc
 */




#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>

int res;

void visit(char* dir, struct dirent** actdir)
{ 
  if(dir != 0)
    printf("%s\n", dir);
  
  int i;
  
  //Data la directory dir, actdir punterà ad un array che contiene le Dirent delle sottodirectory + file in dir
  int n = scandir(dir, &actdir, 0, alphasort);
  for(i = 0; i < n; i++)
  {
    if(actdir[i]->d_type == DT_DIR && strcmp(actdir[i]->d_name, ".") != 0 && strcmp(actdir[i]->d_name, "..") != 0)
    {
      res++;
      printf("%s\n", actdir[i]->d_name);
      int ln = strlen(actdir[i]->d_name);
      char namedir[256];
      strcpy(namedir, dir);
      //Se la dir è la root allora non copio ulteriori /
      if(strcmp(dir, "/") != 0)
	strncat(namedir, "/", 1);
      strncat(namedir, actdir[i]->d_name, ln);
      visit(namedir, 0);
    }
    free(actdir[i]);
  }
  
  free(actdir);
}

int main(int argn, char* args[])
{
  res = 0;
  if(args[1] == 0)
  {
    printf("explor [dir]\n");
    exit(0);
  }
  
  visit(args[1], 0);
  
  printf("Totale directory esplorate: %d\n", res);
  return 0;
}
