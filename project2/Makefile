.SILENT:

default: build

build: clean server client

client:
	gcc -Wall -Wextra -Werror client.c -o client

server:
	gcc -Wall -Wextra  server.c -o server

dist: clean
	mkdir 105032378_project_2
	cp server.c 105032378_project_2/server.c
	cp README 105032378_project_2/README
	cp Makefile 105032378_project_2/Makefile
	tar -czf 105032378.tar.gz 105032378_project_2

clean:	
	rm -rf *tar* client server DS_Store *~ 105032378_project_2