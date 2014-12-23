#ifndef __BOOKADDRESS_H_ 
#define __BOOKADDRESS_H_ 

#define		FIELD_1		256
#define		FIELD_2		32


typedef struct _AddressBook AddressBook;
typedef struct _AddressBook_Bucket AddressBook_Bucket;
typedef struct _AddressBook_collideBucket AddressBook_collideBucket;
typedef struct _AddressBook_FileHeader AddressBook_FileHeader;

typedef struct _AddressBook_EntryIterator
{
  AddressBook_Bucket* current;
} AddressBook_EntryIterator;

//800 byte ogni entry
typedef struct _Entry
{
  char name[FIELD_1];
  char surname[FIELD_1];
  char address[FIELD_1];
  char telephone[FIELD_2];
} Entry;

//Crea un nuovo address book - torna 1 in caso di fallimento
AddressBook* Book_newBook(char* name);

//Carica un nuovo address book - torna 1 in caso di fallimento
AddressBook* Book_loadBook(char* name);

//Inserisce una nuova entry in book, torna 1 in caso di fallimento
int Book_insert(AddressBook* book, const Entry* ent);

/* Posizione un tipo AddressBook_EntryIterator alla entry <name,surname> secondo la ciave di hash, torna il numero di entry con quella chiave di hash
   Torna 0 se non è stato trovato alcun nominativo (fallimento)
*/
int Book_select(AddressBook* book, const char* name, const char* surname, AddressBook_EntryIterator* iter);

/* Restituisce l'entry indicizzata da iter e sposta l'indicizzatore alla prossima entry
 * Torna 0 in caso di successo, 1 in caso di fallimento.
 * I motivi di fallimento sono:
 * 	Errore in lettura del file
 * 	iter non indicizza alcuna entry (iter->current == 0)
 */
int Book_selectNext(AddressBook* book, AddressBook_EntryIterator* iter, Entry* ent);

//Libera le risorse occupate dal AddressBook
void Book_close(AddressBook* book);



#endif
