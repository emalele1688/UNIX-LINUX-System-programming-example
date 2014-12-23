

server: UserBook.o BookAddress.o Streamer.o Server.o main.c
	gcc UserBook.o BookAddress.o Streamer.o Server.o main.c -o ../bin/server

UserBook.o: UserBook.h UserBook.c
	gcc -c UserBook.c

BookAddress.o: BookAddress.h BookAddress.c
	gcc -c BookAddress.c

Streamer.o: Streamer.h Streamer.c
	gcc -c Streamer.c

Server.o: Server.h Server.c
	gcc -c Server.c 
