/*
Author:Jordan Boulanger
Computer Networks - CPSC 5510
Homework 1 - TCP "Finger" Client
*/

/*
References/Credits:
Brain Hall
http://beej.us/guide/bgnet/output/html/multipage/clientserver.html
--This was used as a reference for a basic tcp client server setup
*/

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "10545" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd; 
	char recieveBuffer[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;


	if (argc != 3) {
	    fprintf(stderr,"usage: [proxyserver.com] [port] \n");
	    exit(1);
	}

	std::string host = argv[1];
	std::string port = argv[2];
	

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(host.c_str(), port.c_str(), &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
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
		return 2;
	}
/*
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);*/

	freeaddrinfo(servinfo); // all done with this structure
    
    // elle on 10/24 - user string request w/ debug print statements
    printf ("Enter request: ");
    char  str[545];
    fgets(str, 545, stdin);

    char* request = strtok(str, " ");
    char* url = strtok(NULL, " ");
    char* http_type = strtok(NULL, " ");
    
    printf(request);
    printf("\n");
    printf(url);
    printf("\n");
    printf(http_type);
    // elle on 10/24 - end
	
    int bytesSent = -2;
  	std::string request;
  	printf("Please enter your request as an absolute URI: ");
  	std::getline(std::cin, request);
  	printf("\n%s\n", request.c_str() );
  	int bytesToSend = request.length() + 1;
  	char* sendBuf = new char[bytesToSend];
    strcpy(sendBuf, request.c_str());
    char* bufptr = sendBuf;
    while ((bytesSent = send(sockfd, bufptr, bytesToSend, 0)) > 0){

		bytesToSend = bytesToSend - bytesSent;
		bufptr += bytesSent;
		
	}
	delete[] sendBuf;

	exit(1)/;

	// sent request to server

	int bytesRecieved = -2;
	int bufSpace = MAXDATASIZE-1;
	char* buf = recieveBuffer;
	while((bytesRecieved = recv(sockfd, buf, bufSpace, 0)) > 0){
		// don't need to store the information just print it so we can overwrite on the same buffer
		buf[bytesRecieved] = '\0';
		printf("%s", recieveBuffer);

	}	

	close(sockfd);

	return 0;
}

