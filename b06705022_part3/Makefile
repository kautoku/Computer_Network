
all: client server

client: client.o
	gcc client.o -o client -lpthread

server: server.o
	gcc server.o -o server -lpthread 

client.o: client.c
	gcc -c client.c -lpthread

server.o: server.c
	gcc -c server.c -lpthread

clean: 
	rm client.o client server.o server

