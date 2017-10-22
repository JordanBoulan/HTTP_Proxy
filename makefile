default: server.cpp client.cpp
	g++ -c server.cpp client.cpp
	g++ -o client client.o
	g++ -o server server.o
