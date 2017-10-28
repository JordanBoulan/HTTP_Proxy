	
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
#include <vector>
using namespace std;

int main(int argc, char *argv[])
{
	// variable delcaration
	char input_str[545];
	int size = 3;
	int index = 3;
	vector<char*> lines(size);

	// user request prompt
	printf ("Enter request: ");
    
    // inputs request
    fgets(input_str, 545, stdin);

    // parsing first line of input request
	char* request = strtok(input_str, " ");
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

    return 0;
}	