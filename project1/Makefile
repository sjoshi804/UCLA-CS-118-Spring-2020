.SILENT:

default: build

build: clean server

client:
	gcc -g -Wall -Wextra -Werror -pthread client.c -o client

server:
	gcc -g -Wall -Wextra -Werror -pthread server.c -o server

dist: 
	tar -czf 105032378.tar.gz server.c README Makefile

clean:	
	rm -rf *tar* client server *.o *.dSYM DS_Store *~ 