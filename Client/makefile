all: Client
Client: Client.o
	g++ -g  Client.o -o Client  -lpthread

Client.o: Client.cpp Utils.h
	g++ -c Client.cpp -lpthread

clean:
	rm -f *.o Client
