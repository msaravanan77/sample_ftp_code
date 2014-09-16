
all:	client server

client:	client.o
	gcc -g -o client -Wall client.o

client.o: client.c cliser.h
	gcc -g -c -o client.o -Wall client.c

server:	server.c server.h cliser.h userauthentication.c
	gcc -g -lpam -ldl -lpam_misc -Wall -o server server.c userauthentication.c

clean:
	rm -f client server client.o server.o userauthentication.o

