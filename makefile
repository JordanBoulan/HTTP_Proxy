default: server.cpp client.cpp
	g++ -c server.cpp client.cpp
	g++ -o client client.o
	g++ -o server server.

build:
	g++ client.cpp -o client
	g++ server.cpp -o server

client:
	./client

server:
	./server

scrap:
	g++ scrap.cpp -o scrap
	./scrap