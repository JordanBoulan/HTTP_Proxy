/*
Author:Jordan Boulanger & Morgan Weaver
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
#include <string.h>
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

std::vector<std::string> split(const char *str, char c = ' ')
{
    std::vector<std::string> result;

    do
    {
        const char *begin = str;

        while(*str != c && *str)
            str++;

        result.push_back(std::string(begin, str));
    } while (0 != *str++);

    return result;
}

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

int main(int argc, char* argv[])
{
	if (argc != 2){
		printf("Bad args. Usage: <portNumber>\n");
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
	std::string client_request = "";
    std::vector <std::string> client_args;

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
					client_request += bufptr[i];
				}
				// storing it in a string handles memory so we dont have to worry about buffer overflow.
				// we can now reuse/overwrite on the same buffer
				
				if (bufptr[bytesRecieved-1] == '\0') //look for null character indicating the end of the cstring. it will be the last thing in the buffer. 
					break; // we have recieved the whole username (application protocal).
                std::cout << "RECEIVED:" << client_request << "\n";
			}


            int sz = client_request.size();
            char rqst_arr[sizeof(client_request)];
            for (int a=0;a<=sz;a++)
            {
                rqst_arr[a]=client_request[a];
            }
           client_args = split(rqst_arr, ' '); //now we have a vector of user args.  Should be 3.

           std::string client_rqst_type = rqst_arr[0];
           std::string client_host = rqst_arr[1];
           std::string client_HTTP_version = rqst_arr[2];
           //If more than 3 args, SEND ERR MSG TO CLIENT
           std::string client_port = "80";
      
           //TEST CODE: FAKE sender response
      		std::string test_response = "HTTP/1.0 200 OK \nDate: Fri, 
            													31 Dec 1999 23:59:59 GMT\nContent-Type: 
      																text/html\nContent-Length: 1354";
      		std::string searchterm = "HTTP/1.0";
      	  std::cout << "\n response: " << sender_response;
      		std::size_t found = test_response.find(searchterm);
  				if (found!=std::string::npos)
    				std::cout << "\nerror code is:  " << test_response.substr(found+8,3) << '\n';
					//Now just forward the response to the client
           
//			dup2(new_fd, 1);
//			dup2 (new_fd, 2);
//      close(new_fd);
//			execl("/bin/finger", "finger", client_request.c_str(), NULL);
			
		}
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}