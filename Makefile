CC=g++
CFLAGS= -I . -g -Wall

client: client.cpp create_requests.cpp helpers.cpp buffer.cpp utils.cpp
	$(CC) -o client client.cpp create_requests.cpp helpers.cpp buffer.cpp utils.cpp $(CFLAGS)

run: client
	./client

clean:
	rm -f *.o client
