/*
Author:Jordan Boulanger
Computer Networks - CPSC 5510
Homework 1 - TCP "Finger" Server
*/

/*
References/Credits:
Brain Hall
http://beej.us/guide/bgnet/output/html/multipage/clientserver.html
--This was used as a reference for a basic tcp client server setup
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <iostream>
#include <vector>


#define BACKLOG 10	 // how many pending connections queue will hold
#define MAXDATASIZE 100 // max number of bytes we can get at once 


void sigchld_handler(int s)
{
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int* getNewSocket(char* host, char* port){

	int sockfd; // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	int rv;
	char recieveBuffer[MAXDATASIZE];
	std::string user = "";

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(host,port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return NULL;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;

		int* request_fd = &sockfd;
		return request_fd;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}


}

int main(int argc, char* argv[])
{
	if (argc != 2){
		printf("Argument error! Usage: server portNumber\n");
		exit(1);
	}
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	int rv;
	char recieveBuffer[MAXDATASIZE];
	std::string user = "";

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1)
		{
			perror("accept error"); // this is where i lost points for not having this check, im dumb
			exit(1);
		}
	

		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener
			int bytesRecieved = -2;
			int bufSpace = MAXDATASIZE-1;
			char* bufptr = recieveBuffer;
			
			while((bytesRecieved = recv(new_fd, bufptr, bufSpace, 0)) > 0){
				
				for (int i = 0; i < bytesRecieved; i++){
					user += bufptr[i]; 
				}
				// storing it in a string handles memory so we dont have to worry about buffer overflow.
				// we can now reuse/overwrite on the same buffer
			
				if (user.at(user.length()-1) == '\n') 
					break; 

			}

			printf("Request: %s\n", user.c_str());

			int size = 3;
			int index = 3;
			std::vector<char*> lines(size);
			char* request_in = new char[user.length()+1];
			strcpy(request_in, user.c_str());

			
		    // parsing first line of input request
			char* request = strtok(request_in, " ");
			char* url = strtok(NULL, " ");
			char* http_type = strtok(NULL, " ");

			// adding parse to vector
			lines[0] = request;
			lines[1] = url;
			lines[2] = http_type;

			// recieves additional lines
			char* line = strtok(NULL, " ");
			while(line){
				// resize function
				if(index == lines.size()){
					size = size * 2;
					lines.resize(size);	
				}
				lines[index] = line;
				line = strtok(NULL, " ");
				index += 1;
			}

			// DEBUG - print out of vector
			for (int i = 0; i < lines.size(); i++){
				printf("%d%s%s\n", i, " : ", lines[i]);
			}

			delete[] request;
			exit(1);

			dup2(new_fd, 1);
			dup2 (new_fd, 2);
            close(new_fd);
			execl("/bin/finger", "finger", user.c_str(), NULL);
			
		}
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}

