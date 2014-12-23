#ifndef __USERBOOK_H_ 
#define __USERBOOK_H_


#define USERNAME_SIZE	128
#define PASSWORD_SIZE	16
#define USER_RD		49
#define USER_RDWR	50


typedef struct _UserBook UserBook;
typedef struct _UserNode UserNode;

typedef struct _UserNode
{
  char user_name[128];
  char user_pass[16];
  char user_mask;
} UserNode;


/* Crea un nuovo user book ed inserisce all'interno i nomi utente chiesti in input.
 * Salva tutto su disco e restituisce un UserBook.
 * Torna -1 (casting a Users*) in caso di problemi.
 */
UserBook* UserBook_newBook(const char* bookName);

/* Carica un nuovo user book ed inserisce all'interno eventuali nuovi nomi utente chiesti in input.
 * Salva tutto su disco e restituisce un UserBook.
 * Torna -1 (casting a Users*) in caso di problemi.
 */
UserBook* UserBook_loadBook(const char* bookName);

/* Data la coppia <nome,password> restituisce la maschera dei permessi dell'utente.
 * Se nessun utente viene trovato, o la password è errata torna un byte a 0
 */
char UserBook_findUser(UserBook* book, const char* name, const char* password);

void UserBook_printAllUsers(UserBook* book);

void UserBook_close(UserBook* book);


#endif
