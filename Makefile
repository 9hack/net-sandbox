LIBS=-lpthread -lboost_system -lboost_thread-mt
FLAGS=-std=c++11

all: client server

client: client.cpp
	g++ -g -Wall $(FLAGS) $(LIBS) -o client client.cpp 

server: server.cpp
	g++ -g -Wall $(FLAGS) $(LIBS) -o server server.cpp -lpthread

clean:
	rm -f client server *.o
