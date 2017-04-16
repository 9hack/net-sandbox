LDLIBS=-I /opt/local/include/ -L/opt/local/lib -lboost_system -Wl,-rpath,/opt/local/lib
FLAGS=-lpthread -lboost_thread

all: client server

client: client.cpp
	g++ -g -Wall $(LDLIBS) -o client client.cpp -lpthread -lboost_thread

server: server.cpp
	g++ -g -Wall $(LDLIBS) -o server server.cpp -lpthread

clean:
	rm -f client server *.o
