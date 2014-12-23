#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
  char nome[30];
  char cognome[30];
  int stipendio;
} 
Persona;

Persona *data;
int size;

void newRubrica();
void insert();
void loadRubrica();
int load(char* path);
int save(char* path);
void leggi();

int main(int arg, char *args[])
{
  char scelta[8];
  size = 0;
  data = NULL;
  
  while(1)
  {
    printf("Opzioni disbonibili:\n");
    printf("\tnuovo\n");
    printf("\tcarica\n");
    printf("\tleggi\n");
    printf("\tquit\n");
    fgets(scelta, 8, stdin);
    
    if(strncmp(scelta, "nuovo", 4) == 0)
      newRubrica();
    if(strncmp(scelta, "carica", 5) == 0)
      loadRubrica();
    if(strncmp(scelta, "leggi", 4) == 0)
      leggi();
    if(strncmp(scelta, "quit", 3) == 0)
      break;
  }
  
  free(data);
  
  return 0;
}

void newRubrica()
{
  if(data == NULL)
    data = calloc(128, sizeof(Persona));
  
  insert();
  
  char risposta[2];
  printf("Vuoi salvare i dati? (si/no)\n");
  fgets(risposta, 2, stdin);
  while(getchar() != '\n');
  if(strncmp(risposta, "si", 1) == 0)
  {
    char path[64];
    printf("Nome file?\n");
    fgets(path, 64, stdin);
    if(save(path) == -1)
      printf("File non salvato\n");
    else
      printf("Salvato con successo\n");
  }
}

void leggi()
{
  int i = 0;
  for(i = 0; i < size; i++)
  {
    printf("Nome:");
    fputs(data[i].nome, stdout);
    printf("Cognome:");
    fputs(data[i].cognome, stdout);
    printf("Numero: %d\n", data[i].stipendio);
  }
}

void insert()
{
  char scelta[2];
  do
  {
    printf("Nome:\n");
    fgets(data[size].nome, 30, stdin); 
    printf("Cognome\n");
    fgets(data[size].cognome, 30, stdin);
    printf("numero:\n");
    scanf("%d", &data[size].stipendio);
    while(getchar() != '\n');
    
    size++;
    
    printf("Altre persone?(si/no)\n");
    fgets(scelta, 2, stdin); 
    while(getchar() != '\n');
  }
  while(strncmp(scelta, "si", 1) == 0);
}

void loadRubrica()
{
  if(data != NULL)
    free(data);
  data = calloc(128, sizeof(Persona));
  
  char path[64];
  printf("Inserisci il nome del file da caricare\n");
  fgets(path, 64, stdin);

  if(load(path) == -1)
    printf("File non caricato\n");
}

int load(char* path)
{
  FILE* in;
  in = fopen(path, "rb");
  if(in == NULL)
  {
    printf("Impossibile caricare il file, esiste?\n");
    return -1;
  }
  
  int i = 0;
  while(1)
  {
    if(fread(&data[i], sizeof(Persona), 1, in) != 1)
    {
      if(feof(in))
	break;
      printf("Impossibile leggere sul file, protetto da lettura?\n");
      return -1;
    }
    i++;
  }
  
  size = i;
  
  if(fclose(in) == EOF)
    printf("Impossibile chiudere lo stream verso il file\n");
  
  return 0;
}

int save(char* path)
{
  FILE* out;
  out = fopen(path, "wb");
  if(out == NULL)
  {
    printf("Errore nell'apertura del file, controllare che non sia protetto da scrittura\n");
    return -1;
  }
  
  int i = 0;
  for(i = 0; i < size; i++)
  {
    if(fwrite(&data[i], sizeof(Persona), 1, out) != 1)
    {
      printf("Errore in scrittura\n");
      return -1;
    }
  }
  
  if(fclose(out) == EOF)
    printf("Impossibile chiudere lo stream verso il file\n");
  
  return 0;
}
