server: server.c
	gcc -o server server.c -lssl -lcrypto

client: client.c
	gcc -o client client.c -lssl -lcrypto

all: server client

clean:
	rm -f server client