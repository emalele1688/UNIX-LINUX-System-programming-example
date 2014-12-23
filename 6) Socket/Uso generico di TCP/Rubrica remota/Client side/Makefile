 
client: Client.o Streamer.o
	gcc Client.o Streamer.o -o ../bin/client

Client.o: Client.c
	gcc -c Client.c

Streamer.o: Streamer.c Streamer.h
	gcc -c Streamer.c