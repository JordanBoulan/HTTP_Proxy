/*

Author:Jordan Boulanger, Ellery Baines, Morgan Weaver
Computer Networks - CPSC 5510
Project 2 - http proxy part 1
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
#include <pthread.h>
#include <iostream>
#include <vector>
#include <time.h>


#define BACKLOG 30	 // how many pending connections queue will hold
#define MAXDATASIZE 100



void sigchld_handler(int s)
{
	int saved_errno = errno;
	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}
void sigpipe_handler(int s){

}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int sendall(int s, char *buf, int *len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;

    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n==-1?-1:0; // return -1 on failure, 0 on success
} 

void sendResponseToClient(std::string serverResponse, int new_fd){
	int bytesToSend2 = serverResponse.length() + 1;
	int bytesSent2 = 0;
	char* response_tosend = new char[bytesToSend2];
	strcpy(response_tosend, serverResponse.c_str());  	
	char* bufptr2 = response_tosend;
		 	
	while ((bytesSent2 = send(new_fd, bufptr2, bytesToSend2, 0)) > 0){
		bytesToSend2 = bytesToSend2 - bytesSent2;
		bufptr2 += bytesSent2;
		if (bytesToSend2 <1){
			break;	
		}
		delete[] response_tosend;
	}
	close(new_fd);	
}



void* doRequest(void* in){

			int bytesRecieved = -2;
			int bufSpace = MAXDATASIZE-1;
			char recieveBuffer[MAXDATASIZE];
			std::string request_input = "";
			char* bufptr = recieveBuffer;
			int new_fd = *((int*) in);

			
			
			while((bytesRecieved = recv(new_fd, bufptr, bufSpace, 0)) > 0){

				for (int i = 0; i < bytesRecieved; i++){
					request_input += bufptr[i]; 
				}
				memset(bufptr, 0, MAXDATASIZE-1);
				// storing it in a string handles memory so we dont have to worry about buffer overflow.
				// we can now reuse/overwrite on the same buffer
			
				if (request_input.length() > 3 && request_input.at(request_input.length()-1) == '\n' && request_input.at(request_input.length()-2) == '\r' && request_input.at(request_input.length()-3) == '\n' && request_input.at(request_input.length()-4) == '\r') 
					break; 

			}

			
			char* request_in = new char[request_input.length()+1];
			strcpy(request_in, request_input.c_str());
			int size = 3;
			int index = 3;
			std::vector<char*> lines(size);
			std::string serverResponse = "";
			
		    
		    // parsing first line of input request
			char* request = strtok(request_in, " ");
			
			if (request == NULL){
				    serverResponse += "HTTP/1.0 500 error: Internal Server error\r\n";
					sendResponseToClient(serverResponse, new_fd);
					return NULL;

			}

			char* url = strtok(NULL, " ");

			if (url == NULL){
				    serverResponse += "HTTP/1.0 500 error: Internal Server error\r\n";
					sendResponseToClient(serverResponse, new_fd);
					return NULL;
			}

			char* http_type = strtok(NULL, "\r\n");

			// adding parse to vector
			lines[0] = request;
			lines[1] = url;
			lines[2] = http_type;

			// recieves additional lines
			char* line = strtok(NULL, "\r\n");
			while(line){
				// resize function
				if((unsigned) index == lines.size()){
					size = size + 1;
					lines.resize(size);	
				}
				lines[index] = line;
				line = strtok(NULL, "\r\n");
				index += 1;
			}

			lines.pop_back(); // remove blank entry
			index = index - 1;


			bool isFormated = true;
			std::string request_formatted = "";
			std::string unformatted = lines[1];
			
			int start = unformatted.find( "//", 0);

			if ((unsigned)start == std::string::npos)
				start = -2;

			int relitive_start = unformatted.find( "/", start+2);
			int port_specified = unformatted.find(":", start+2);
			std::string host;
			std::string path;
			std::string port;
			if (relitive_start != -1){

				if (port_specified != -1){
					port = unformatted.substr(port_specified + 1, relitive_start- (port_specified+1));
					path = unformatted.substr(relitive_start, std::string::npos);
					host = unformatted.substr(start+2, port_specified-(start+2));

				}
				
				else{
					port = "80";
					path = unformatted.substr(relitive_start, std::string::npos);
					host = unformatted.substr(start+2, relitive_start-(start+2));
\
				}

			}
			else{
				if (port_specified != -1){
					port = unformatted.substr(port_specified + 1, relitive_start- (port_specified+1));
					path = "/";
					host = unformatted.substr(start+2, port_specified-(start+2));

				}
				
				else{
					port = "80";
					path = "/";
					host = unformatted.substr(start+2, std::string::npos);

				}

				
			}

			
			
			request_formatted += "GET ";
			request_formatted += path;
			request_formatted += " HTTP/1.0\r\n";
			if (host.find("www", 0) == std::string::npos)
				request_formatted += "Host:" "www."+ host + "\r\n";
			else
			request_formatted += "Host:" + host + "\r\n";
			request_formatted += "Connection:close\r\n";
			
			

			
			

			for(int i = 3; i < index; i++){
				unformatted = lines[i];
				if (unformatted.find("Connection", 0) != std::string::npos)
					continue;
				if (unformatted.find("Host", 0) != std::string::npos)
					continue;
				
				
				request_formatted += lines[i]; 
				request_formatted += "\r\n";

			}

			// error checks and server response
			
			// request check

			request_formatted += "\r\n";

			if (strcmp(lines[0], "GET") != 0){
				serverResponse += "HTTP/1.0 501 Error: Request type not supported\r\n";
				sendResponseToClient(serverResponse, new_fd);
				return NULL;
			}
			// options formatting check
			else if (!isFormated){
				serverResponse += "HTTP/1.0 500 Error: Internal Server error\r\n";
				sendResponseToClient(serverResponse, new_fd);
				return NULL;
			}
			// proxy response piping
			else{
				int request_fd;
				struct addrinfo hints1, *servinfo1, *p1;
				int rv1;

				memset(&hints1, 0, sizeof hints1);
				hints1.ai_family = AF_UNSPEC;
				hints1.ai_socktype = SOCK_STREAM;
				hints1.ai_flags = AI_PASSIVE; // use my IP

				
				if ((rv1 = getaddrinfo(host.c_str(), port.c_str(), &hints1, &servinfo1)) != 0) {
					serverResponse += "HTTP/1.0 500 error: Internal Server error\r\n";
					sendResponseToClient(serverResponse, new_fd);
					return NULL;
				}

				// loop through all the results and connect to the first we can
				for(p1 = servinfo1; p1 != NULL; p1 = p1->ai_next) {
					if ((request_fd = socket(p1->ai_family, p1->ai_socktype,
							p1->ai_protocol)) == -1) {
						serverResponse += "HTTP/1.0 500 error: Internal Server error\r\n";
						sendResponseToClient(serverResponse, new_fd);
					
						return NULL;
					}

					if (connect(request_fd, p1->ai_addr, p1->ai_addrlen) == -1) {
						serverResponse += "HTTP/1.0 500 error: Internal Server error\r\n";
						sendResponseToClient(serverResponse, new_fd);
						close(request_fd);
						
						return NULL;
					}

					break;
				}

				if (p1 == NULL) {
					serverResponse += "HTTP/1.0 500 error: Internal Server error\r\n";
					sendResponseToClient(serverResponse, new_fd);
					
					return NULL; // will modularize in next version
				}

				freeaddrinfo(servinfo1); // all done with this structure


				
			    

			

				int bytesToSend1 = request_formatted.length();
				char* final_format = new char[bytesToSend1];
				memcpy(final_format, request_formatted.c_str(), request_formatted.length());
				int bytesSent1 = 0;
			  	
			    char* bufptr1 = final_format;
			   
			 	
			    while ((bytesSent1 = send(request_fd, bufptr1, bytesToSend1, MSG_DONTWAIT)) > 0){

					bytesToSend1 = bytesToSend1 - bytesSent1;
					bufptr1 += bytesSent1;
					if (bytesToSend1 < 1){
						break;
					}
					
				}
				
			
				delete[] final_format;

				
	
				bufptr = recieveBuffer;
				memset(bufptr, 0, MAXDATASIZE-1);
				bufSpace = MAXDATASIZE-1;
				bool keepgoing = true;
				while (keepgoing)
				{
					bytesRecieved = recv(request_fd, bufptr, bufSpace, 0);
					if (bytesRecieved == 0)
						break;
					if (bytesRecieved == -1)
						break;

					

					
					int val = sendall(new_fd, recieveBuffer, &bytesRecieved);
					if (val == -1)
						break;

					memset(bufptr, 0, MAXDATASIZE-1);

		
		}
			close(request_fd);
}
	
		

		
		close(new_fd);

		
        delete[] request_in;
          
        return NULL;
}

int main(int argc, char* argv[])
{
	if (argc != 2){
		printf("Argument error! Usage: proxy portNumber\n");
		exit(1);
	}
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa, sa_pipe;
	int yes=1;
	int rv;
	pthread_t mythread;


	


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

	sa_pipe.sa_handler = sigpipe_handler; // reap all dead processes
	sigemptyset(&sa_pipe.sa_mask);
	if (sigaction(SIGPIPE, &sa_pipe, NULL) == -1) {
		perror("sigpipe");
		exit(1);
	}



	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
	
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1)
		{
			//perror("accept error");
			continue;
		}
	
		
		
		 int* arg = &new_fd;
	 
		 pthread_create(&mythread, NULL, doRequest, (void*) arg);
		 pthread_detach(mythread); //can now reuse thread id
		 
		
	}
      		
		
		
	

	return 0;
}

