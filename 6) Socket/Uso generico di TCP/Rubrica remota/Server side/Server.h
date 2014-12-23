#ifndef __SERVER_H_
#define __SERVER_H_

#include <stdint.h>
#include "BookAddress.h"
#include "UserBook.h"

#define MAX_PENDING_CONNECTION		3
#define TIMEOUT_SECOND			60


typedef struct _Server Server;

Server* Server_initServer(uint16_t portNumber);

void Server_setUserBook(Server* server, UserBook* book);

void Server_setAddressBook(Server* server, AddressBook* book);

int Server_execute(Server* server);

void Server_close(Server* server);


#endif