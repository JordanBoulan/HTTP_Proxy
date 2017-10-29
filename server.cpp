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

int* getNewSocket(const char* host, const char port[]){

	int sockfd; // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	int yes=1;
	int rv;
	
	std::string user = "";
	printf("host: %s port: %s\n",host, port );

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	
	if ((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return NULL;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {

			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return NULL;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	int* request_fd = &sockfd;
	return request_fd;


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
			//perror("accept error"); // this is where i lost points for not having this check, im dumb
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
			
				if (user.at(user.length()-1) == '\n' && user.at(user.length()-2) == '\r' && user.at(user.length()-3) == '\n' && user.at(user.length()-4) == '\r') 
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
				if((unsigned) index == lines.size()){
					size = size * 2;
					lines.resize(size);	
				}
				lines[index] = line;
				line = strtok(NULL, " ");
				index += 1;
			}

			// DEBUG - print out of vector
			for (int i = 0; (unsigned) i < lines.size(); i++){
				printf("%d%s%s\n", i, " : ", lines[i]);
			}


				std::string request_formatted = "";
				std::string unformatted = lines[1];
				
				int start = unformatted.find( "//", 0);
				int relitive_start = unformatted.find( "/", start+2);
				std::string host;
				std::string path;
				if (relitive_start != -1){
					path = unformatted.substr(relitive_start, std::string::npos);
					host = unformatted.substr(start+2, relitive_start-(start+2));
				}
				else{
					host = unformatted.substr(start+2, std::string::npos);
					path = "/";
				}
				
				request_formatted += "GET ";
				request_formatted += path;
				request_formatted += " HTTP/1.0\r\n";
				request_formatted += "Host:" + host + "\r\n";
				request_formatted += "Connection:close \r\n\r\n";

				for (int i = 3; i < index; i++){
					request_formatted += lines[i];
				}

				printf("host: %s formatted: %s\n", host.c_str(), request_formatted.c_str() );


			std::string serverResponse = "";
			if (strcmp(lines[0], "GET") != 0){
				serverResponse += "501 Error: Request type not supported";
			}
			
			else{
				int request_fd; // listen on sock_fd, new connection on new_fd
				struct addrinfo hints1, *servinfo1, *p1;
				int yes1=1;
				int rv1;

				memset(&hints1, 0, sizeof hints1);
				hints.ai_family = AF_UNSPEC;
				hints.ai_socktype = SOCK_STREAM;
				hints.ai_flags = AI_PASSIVE; // use my IP

				
				if ((rv1 = getaddrinfo(host.c_str(), "80", &hints1, &servinfo1)) != 0) {
					fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv1));
					return 1;
				}

				// loop through all the results and connect to the first we can
				for(p1 = servinfo1; p1 != NULL; p1 = p1->ai_next) {
					if ((request_fd = socket(p1->ai_family, p1->ai_socktype,
							p1->ai_protocol)) == -1) {

						continue;
					}

					if (connect(request_fd, p1->ai_addr, p1->ai_addrlen) == -1) {
						close(sockfd);
						continue;
					}

					break;
				}

				if (p1 == NULL) {
					fprintf(stderr, "client: failed to connect\n");
					return 1;
				}

				freeaddrinfo(servinfo1); // all done with this structure

				if (p1 == NULL)  {
					fprintf(stderr, "server: failed to bind\n");
					exit(1);
				}


			
				int bytesToSend1 = request_formatted.length() +1;
				char* final_format = new char[bytesToSend1];
				strcpy(final_format, request_formatted.c_str());
				int bytesSent1 = 0;
			  	
			    char* bufptr1 = final_format;
			    printf("%s\n", final_format );
			 	
			    while ((bytesSent1 = send(request_fd, bufptr1, bytesToSend1, 0)) > 0){

					bytesToSend1 = bytesToSend1 - bytesSent1;
					bufptr1 += bytesSent1;
					
				}
				
			

				bufptr = recieveBuffer;
				bufSpace = MAXDATASIZE-1;
				while((bytesRecieved = recv(request_fd, bufptr, bufSpace, 0)) > 0){
					serverResponse += bufptr;
				}
				serverResponse += "\n";
			
			
		}

			int bytesToSend2 = serverResponse.length() + 1;
			int bytesSent2 = 0;
			char* response_tosend = new char[bytesToSend2];
			strcpy(response_tosend, serverResponse.c_str());
		  	
		    char* bufptr2 = response_tosend;
		 	
		    while ((bytesSent2 = send(new_fd, bufptr2, bytesToSend2, 0)) > 0){

				bytesToSend2 = bytesToSend2 - bytesSent2;
				bufptr2 += bytesSent2;
				
			}
			

			
          delete[] request;
          //delete[] final_format;
          delete[] response_tosend;			
		}
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}

