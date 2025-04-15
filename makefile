.PHONEY: all clean

all: server client

server: server.cpp
	@g+ -std=c++11 -o server server.cpp

client: clien.cpp
	@g++ -std=c++ -o client client.cpp

clean:
	@rm -f server client