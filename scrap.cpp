	
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


int first_loved(char* stringee, char unloved) {
	int index = 0;
	char next;

	// finding first location without unloved char
	while(stringee[index] == unloved) {
		index += 1;
	}
	return index;
}

int last_loved(char* stringee, char unloved) {
	int index = 0;
	char next = stringee[index];

	// finding length
	while(next != '\n') {
		printf("%c\n", &next);
		index += 1;
		next = stringee[index];
	}
	printf("%d\n", index);

	while(next == unloved){
		index -= 1;
		next = stringee[index];
	}
	printf("%d\n", index);

	return index;
}

int main(int argc, char *argv[])
{
	// variable delcaration
	int str_length = 545;
	char input_str[str_length];
	int size = 3;
	int index = 3;
	char BLANKSPACE = ' ';
	char ENDLINE = '\n';
	int start, end = -1;
	vector<char*> lines(size);

	// user request prompt
	printf ("Enter request: ");
    
    // inputs request
    fgets(input_str, 545, stdin);

	// trimming leading and trailing spaces
	// start = first_loved(input_str, BLANKSPACE);
	// end = last_loved(input_str, BLANKSPACE);
	// printf("START:%c\n", input_str[start]);
	// printf("END  :%c\n", input_str[end]);

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

	// reconstruct lines into a single string
	string full_string;
	for(int i = 0; i < index; i++){
		full_string = full_string + lines[i] + BLANKSPACE;
	}

	// DEBUG - print outs
	for (int i = 0; i < lines.size(); i++){
		printf("%d%s%s\n", i, " : ", lines[i]);
	}

	printf("full_string : %s\n", full_string.c_str()); 

    return 0;
}	