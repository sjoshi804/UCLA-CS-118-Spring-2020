.SILENT:

default: build

build: clean server

client:
	gcc -g -Wall -Wextra -Werror -pthread client.c -o client

server:
	gcc -g -Wall -Wextra -Werror -pthread server.c -o server

dist: 
	mkdir 105032378_project_1
	cp server.c 105032378_project_1/server.c
	cp README 105032378_project_1/README
	cp Makefile 105032378_project_1/Makefile
	tar -czf 105032378.tar.gz 105032378_project_1

clean:	
	rm -rf *tar* client server *.o *.dSYM DS_Store *~ 105032378_project_1