LDLIBS=-I /opt/local/include/ -L/opt/local/lib -lboost_system -Wl,-rpath,/opt/local/lib
FLAGS=-lpthread -lboost_thread

all: game_client echo_server game_server server waitress

game_client: game_client.o
	g++ -g -Wall $(LDLIBS) -o game_client game_client.cpp $(FLAGS)

game_server: game_server.o
	g++ -g -Wall $(LDLIBS) -o game_server game_server.cpp $(FLAGS)
	
waitress: waitress.o
	g++ -g -Wall $(LDLIBS) -o waitress waitress.cpp $(FLAGS)

server: server.cpp
	g++ -g -Wall $(LDLIBS) -o server server.cpp $(FLAGS)

client: client.o
	g++ -g -Wall $(LDLIBS) -o client client.cpp $(FLAGS)
	
echo_server: echo_server.cpp
	g++ -g -Wall $(LDLIBS) -o echo_server echo_server.cpp $(FLAGS)

echo_client: echo_client.o
	g++ -g -Wall $(LDLIBS) -o echo_client echo_client.cpp $(FLAGS)

clean:
	rm -f server client echo_server echo_client *.o
