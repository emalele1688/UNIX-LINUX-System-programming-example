#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "UserBook.h"
#include "Streamer.h"

#define HASH_SCALE_FACT		2


static int FLAG_TERM;

typedef struct _UserBook
{
  UserNode* head;
  unsigned int entryCount;
} UserBook;


int UserBook_insert(const UserNode* insertNode, UserNode* bookHead, int maxDim);
int UserBook_computeHashCode(const char* str, int length);

void UserBook_term(int sig)
{
  FLAG_TERM = 1;
}

/* Crea un nuovo user book ed inserisce all'interno i nomi utente chiesti in input.
 * Salva tutto su disco e restituisce un UserBook.
 * Torna -1 (casting a Users*) in caso di problemi.
 */
UserBook* UserBook_newBook(const char* bookName)
{
  UserBook* pbook;
  struct sigaction exitAction;
  struct sigaction oldIntAction;
  struct sigaction oldQuitAction;
  struct sigaction oldTermAction;
  UserNode temp;
  int size, ds_file, i;
  
  FLAG_TERM = 0;
  
  //Installo un gestore per i segnali di terminazione validi per questa routine
  sigemptyset(&exitAction.sa_mask);
  sigaddset(&exitAction.sa_mask, SIGINT);
  sigaddset(&exitAction.sa_mask, SIGQUIT);
  sigaddset(&exitAction.sa_mask, SIGTERM);
  exitAction.sa_flags = 0;
  exitAction.sa_handler = UserBook_term;
  if(sigaction(SIGINT, &exitAction, &oldIntAction) == -1 || sigaction(SIGQUIT, &exitAction, &oldQuitAction) == -1 || sigaction(SIGTERM, &exitAction, &oldTermAction) == -1)
    fprintf(stderr, "Problemi nell'installare un gestore per i tre segnali di uscita: %s\n", strerror(errno));
  
  pbook = malloc(sizeof(UserBook));
  if(pbook == (UserBook*)0)
  {
    printf("Impossibile creare uno UserBook: %s\n", strerror(errno));
    return (UserBook*)0;
  }
  memset(pbook, 0, sizeof(UserBook)); 
  
  printf("Numero di utenti da inserire nel book?\n");
  scanf("%d", &size);
  
  pbook->head = malloc(size * sizeof(UserNode));
  if(pbook->head == (UserNode*)0)
  {
    printf("Impossibile inserire nominativi nello UserBook: %s\n", strerror(errno));
    free(pbook);
    return (UserBook*)0;
  }
  memset(pbook->head, 0, size * sizeof(UserNode));
  
  pbook->entryCount = size;
  
  ds_file = open(bookName, O_CREAT | O_RDWR | O_EXCL, 0660);
  if(ds_file == -1)
  {
    printf("Impossibile creare un file sul disco per salvere lo UserBook: %s\n", strerror(errno));
    free(pbook->head);
    free(pbook);
    return (UserBook*)0;
  }
  
  //Lascio spazio per l'header
  lseek(ds_file, sizeof(int), SEEK_SET);
  
  for(i = 0; i < size; i++)
  {
    //Posso uscire solo se gli ultimi dati inseriti sono stati scritti sul file
    if(FLAG_TERM == 1)
    {
      //Scrivo l'header prima di termianare
      lseek(ds_file, 0, SEEK_SET);
      writeToStream(ds_file, &i, sizeof(int));
      free(pbook->head);
      free(pbook);
      close(ds_file);
      return (UserBook*)0;
    }
    
    memset(&temp, 0, sizeof(UserNode));
    printf("Utente numero %d\n", i+1);
    
    while(getchar() != '\n');
    printf("\tNome utente:\n");
    scanf("%s", temp.user_name);
    //temp.user_name[strlen(temp.user_name)-1] = '\0';
    
    while(getchar() != '\n');
    printf("\tPassword:\n");
    scanf("%s", temp.user_pass);
    //temp.user_pass[strlen(temp.user_pass)-1] = '\0';
    
    while(getchar() != '\n');
    printf("\tPermessi: (1 = sola lettura ; 2 = lettura e scrittura\n");
    scanf("%c", &temp.user_mask);
    
    //Inserisci temp sul file
    if( writeToStream(ds_file, &temp, sizeof(UserNode)) < sizeof(UserNode))
      printf("Utente %s non salvato su disco\n", temp.user_name);
    
    //Inserisci temp nel book in memoria
    if(UserBook_insert(&temp, pbook->head, size) == 0)
    {
      i--;
      continue;
    }
  }
  
  //Creo header del file 
  lseek(ds_file, 0, SEEK_SET);
  writeToStream(ds_file, &size, sizeof(int));

  close(ds_file);
  
  if(sigaction(SIGINT, &oldIntAction, NULL) == -1 || sigaction(SIGQUIT, &oldQuitAction, NULL) == -1 || sigaction(SIGTERM, &oldTermAction, NULL) == -1)
    fprintf(stderr, "Problemi nell'installare un gestore per i tre segnali di uscita: %s\n", strerror(errno));
  
  if(FLAG_TERM == 1)
  {
    free(pbook->head);
    free(pbook);
    return (UserBook*)0;
  }
  
  return pbook; 
}

/* Carica un nuovo user book ed inserisce all'interno eventuali nuovi nomi utente chiesti in input.
 * Salva tutto su disco e restituisce un UserBook.
 * Torna -1 (casting a Users*) in caso di problemi.
 */
UserBook* UserBook_loadBook(const char* bookName)
{
  UserBook* pbook;
  struct sigaction exitAction;
  struct sigaction oldIntAction;
  struct sigaction oldQuitAction;
  struct sigaction oldTermAction;
  UserNode temp;
  int newsize, actsize = 0, incsize, ds_file, i;
  
  FLAG_TERM = 0;
  //Installo un gestore per i segnali di terminazione validi per questa routine
  sigemptyset(&exitAction.sa_mask);
  sigaddset(&exitAction.sa_mask, SIGINT);
  sigaddset(&exitAction.sa_mask, SIGQUIT);
  sigaddset(&exitAction.sa_mask, SIGTERM);
  exitAction.sa_flags = 0;
  exitAction.sa_handler = UserBook_term;
  if(sigaction(SIGINT, &exitAction, &oldIntAction) == -1 || sigaction(SIGQUIT, &exitAction, &oldQuitAction) == -1 || sigaction(SIGTERM, &exitAction, &oldTermAction) == -1)
    fprintf(stderr, "Problemi nell'installare un gestore per i tre segnali di uscita: %s\n", strerror(errno));
  
  ds_file = open(bookName, O_RDWR);
  if(ds_file == -1)
  {
    printf("Impossibile creare uno UserBook: %s\n", strerror(errno));
    return (UserBook*)0;
  }
  
  readToStream(ds_file, &actsize, sizeof(int));
  
  pbook = malloc(sizeof(UserBook));
  if(pbook == (UserBook*)0)
  {
    printf("Impossibile creare uno UserBook: %s\n", strerror(errno));
    return (UserBook*)0;
  }
  memset(pbook, 0, sizeof(UserBook)); 
  
  printf("Numero di nuovi utenti da inserire nel book? (0 se nessuno)\n");
  scanf("%d", &incsize);

  newsize = actsize + incsize;
  
  pbook->head = malloc(newsize * sizeof(UserNode));
  if(pbook->head == (UserNode*)0)
  {
    printf("Impossibile inserire nominativi nello UserBook: %s\n", strerror(errno));
    free(pbook);
    return (UserBook*)0;
  }
  memset(pbook->head, 0, newsize * sizeof(UserNode));
  
  pbook->entryCount = newsize;
  
  for(i = 0; i < actsize; i++)
  {
    if(FLAG_TERM == 1)
    {
      free(pbook->head);
      free(pbook);
      close(ds_file);
      return (UserBook*)0;
    }
    
    memset(&temp, 0, sizeof(UserNode));
    if( readToStream(ds_file, &temp, sizeof(UserNode)) < sizeof(UserNode))
    {
      printf("Problemi nella lettura di un entry: %s\n", strerror(errno));
      continue;
    }
    
    if(UserBook_insert(&temp, pbook->head, newsize) == 0)
    {
      i--;
      continue;
    }
  }
  
  for(i = 0; i < incsize; i++)
  {
    if(FLAG_TERM == 1)
    {
      //Quantità di entry realmente scritte
      int ns = actsize + i;
      lseek(ds_file, 0, SEEK_SET);
      writeToStream(ds_file, &ns, sizeof(int));
      free(pbook->head);
      free(pbook);
      close(ds_file);
      return (UserBook*)0;
    }
    
    memset(&temp, 0, sizeof(UserNode));
    printf("Utente numero %d\n", i+1);
    
    while(getchar() != '\n');
    printf("\tNome utente:\n");
    scanf("%s", temp.user_name);
    //temp.user_name[strlen(temp.user_name)-1] = '\0';
    
    while(getchar() != '\n');
    printf("\tPassword:\n");
    scanf("%s", temp.user_pass);
    //temp.user_pass[strlen(temp.user_pass)-1] = '\0';
    
    while(getchar() != '\n');
    printf("\tPermessi: (1 = sola lettura ; 2 = lettura e scrittura\n");
    scanf("%c", &temp.user_mask);
    
    //Inserisci temp nel book in memoria
    if(UserBook_insert(&temp, pbook->head, newsize) == 0)
    {
      i--;
      continue;
    }
    
    //Inserisci temp sul file
    if( writeToStream(ds_file, &temp, sizeof(UserNode)) == 0)
      printf("Utente %s non salvato su disco\n", temp.user_name);
  }
  
  lseek(ds_file, 0, SEEK_SET);
  writeToStream(ds_file, &newsize, sizeof(int));
  
  close(ds_file);
  
  if(sigaction(SIGINT, &oldIntAction, NULL) == -1 || sigaction(SIGQUIT, &oldQuitAction, NULL) == -1 || sigaction(SIGTERM, &oldTermAction, NULL) == -1)
    fprintf(stderr, "Problemi nell'installare un gestore per i tre segnali di uscita: %s\n", strerror(errno));
  
  if(FLAG_TERM == 1)
  {
    free(pbook->head);
    free(pbook);
    return (UserBook*)0;
  }
  
  return pbook;
}

int UserBook_insert(const UserNode* insertNode, UserNode* bookHead, int maxDim)
{
  int hashcode, index;
  
  hashcode = UserBook_computeHashCode(insertNode->user_name, strlen(insertNode->user_name));
  index = hashcode % maxDim;
  
  //Gestione collisione in Linear Probing
  while((bookHead+index)->user_mask != 0)
  {
    if(strcmp((bookHead+index)->user_name, insertNode->user_name) == 0)
    {
      printf("Nominativo già esistente\n");
      return 0;
    }
    
    index++;
    if(index == maxDim)
      index = 0;
  }
  
  strcpy((bookHead+index)->user_name, insertNode->user_name);
  strcpy((bookHead+index)->user_pass, insertNode->user_pass);
  (bookHead+index)->user_mask = insertNode->user_mask;
  
  return 1;
}

/* Data la coppia <nome,password> restituisce la maschera dei permessi dell'utente.
 * Se nessun utente viene trovato, o la password è errata torna un byte a 0
 */
char UserBook_findUser(UserBook* book, const char* name, const char* password)
{
  UserNode* head;
  int hashcode, index, curr;
  
  head = book->head;
  hashcode = UserBook_computeHashCode(name, strlen(name));
  index = hashcode % book->entryCount;
  
  curr = index;
  
  while(strcmp((head+index)->user_name, name) != 0)
  {
    index++;
    if(index == book->entryCount)
      index = 0;
    if(curr == index) //Nessun nominativo trovato
      return 0;
  }
    
  //Controllo password
  if(strcmp((head+index)->user_pass, password) != 0)
    return 0;
  else
    return (head+index)->user_mask;
}

int UserBook_computeHashCode(const char* str, int length)
{
  int i;
  int hash = (int)str[0];
  for(i = 1; i < length; i++)
    hash += ((int)str[i]) * (HASH_SCALE_FACT ^ i);
  
  return hash;
}

void UserBook_printAllUsers(UserBook* book)
{
  UserNode* head = book->head;
  int i = 0;
  while(i < book->entryCount)
  {
    printf("%s\n", head->user_name);
    i++;
    head++;
  }
}

void UserBook_close(UserBook* book)
{
  if(book == 0)
    return;
  
  free(book->head);
  free(book);
}