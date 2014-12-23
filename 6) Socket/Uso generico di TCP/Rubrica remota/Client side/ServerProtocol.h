/* Inserisco in questo header le strutture che verranno usate per creare i pacchetti di communicazione */

#ifndef __SERVERPROTOCOL_H_
#define __SERVERPROTOCOL_H_


#define USERNAME_SIZE	128
#define PASSWORD_SIZE	16
#define	FIELD_1		256
#define	FIELD_2		32

typedef struct _packet_login
{
  char user_name[USERNAME_SIZE];
  char user_pass[PASSWORD_SIZE];
} packet_login;

typedef struct _packet_accept
{
  char byteinfo;
} packet_accept;

typedef struct _packet_entry
{
  char name[FIELD_1];
  char surname[FIELD_1];
  char address[FIELD_1];
  char telephone[FIELD_2];
} packet_entry;

typedef struct _packet_operation
{
  packet_entry ent;
  char oper;
} packet_operation;



#endif