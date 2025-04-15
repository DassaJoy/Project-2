.PHONY: all clean

all: server client

server: server.c
	@gcc -std=c11 -o server server.c

client: client.c
	@gcc -std=c11 -o client client.c

clean:
	@rm -f server client