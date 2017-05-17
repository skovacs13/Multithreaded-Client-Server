all: server client

server:
	gcc -g -Wall -pthread netfileserver.c -lpthread -o server

client:
	gcc -Wall libnetfiles.c -o client