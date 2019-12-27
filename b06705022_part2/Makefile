
all: server

server: server.o
	gcc server.o -o server -lpthread 

server.o: server.c
	gcc -c server.c -lpthread

clean: 
	rm server.o server

