#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "libnetfiles.h"

int sockfd = -1;											// file descriptor for our socket
int portno = -1;											// server port to connect to
int n = -1;												// utility variable - for monitoring reading/writing from/to the socket
char bufHolder[256];											// char array to store data going to and coming from the server
struct sockaddr_in serverAddressInfo;									// Super-special secret C struct that holds address info for building our socket
struct hostent *serverIPAddress;
int fp = -1;


void helpSock(char c){

	
	char path[256];
	char flags[2];
	char holdread[256];
	int nl;
	//char* write = NULL;
	//int len = 0;
	if(c == 'o') {
		printf("Enter pathname and access mode (0 for read-only, 1 for write-only, and 2 for read-write): ");
		scanf("%s", path);
		scanf("%s", flags);
		int flag = atoi(flags);
		printf("Path: %s, Flags: %s\n", path, flags);
		netopen(path, flag);

	}else if(c == 'c') {
		if(fp > 0) {
			netclose(fp);		
		}else {
			error("ERROR: Invalid file.");
		}

	}else if(c == 'r') {
		if(fp > 0) {
			printf("Enter number of bytes to read: ");
			scanf("%s", flags);
			netread(fp, holdread, atoi(flags));

		}else {
			error("ERROR: Could not read file.");
		}
	
		

	}else if(c == 'w') {
		if(fp > 0) {
			while ( (nl = getchar()) != '\n' && nl != EOF );
			printf("Enter data and number of bytes to write: ");
			
			fgets (holdread, 150, stdin);
			printf("Write: %s, Len: %i\n", holdread, strlen(holdread));
			scanf("%s", flags);
			netwrite(fp, holdread, atoi(flags));

		}else {
			error("ERROR: Could not read file.");
		}


	}else {
		exit(1);
	}
	printf("Exit sock\n");
    	
}

void error(char *msg){
	perror(msg);
	exit(0);
}

int netopen(const char *pathname, int flags){
	int fd;
	char temp[2];
	printf("%s %i\n", pathname, flags);
	fd = write(sockfd,"o",1);
	printf("%i\n", strlen(pathname));
	char pathnameTemp[strlen(pathname)+1];
	strcpy(pathnameTemp, pathname);
	pathnameTemp[strlen(pathname)] = NULL;
	
	fd = write(sockfd,pathnameTemp,strlen(pathname));
	read(sockfd, temp, 2);
	
		while(atoi(temp) != 0) {
		printf("-");
	}
	
	if(flags == 0){
		fd = write(sockfd, "0", 1);
		printf("open: 0\n");
	}
	else if(flags == 1){
		fd = write(sockfd, "1", 1);
		printf("open: 1\n");
	}
	else if(flags == 2){
		fd = write(sockfd, "2", 1);
		printf("open: 2\n");
	}
	else {
		error("Incorrect flag.\n");
	}
	
	read(sockfd, temp, 2);
	fd = atoi(temp);
	//free(temp);
	fp = fd;
	printf("File descriptor: %i\n",fp);
	return fd;
}

ssize_t netread(int fildes, void *buf, size_t nbyte){
	printf("Bytes: %i, File: %i\n", nbyte, fp);
	char* bufHold[256];
	write(sockfd, "r", 1);
	
	/*int fd = fp;
	char* fdPnt = malloc(20 * sizeof(char));
	sprintf(fdPnt, "%d", fd);
	write(sockfd, fdPnt, strlen(fdPnt));*/

	int written;
	written = nbyte;
	char* writtenPnt = malloc(10 * sizeof(char));
	sprintf(writtenPnt, "%d", written);
	write(sockfd, writtenPnt, strlen(writtenPnt));
	
	read(sockfd, bufHold, 256);
	if(strlen(bufHold) >= 0) {
		printf("Data read: %s, Number of bytes read: %i\n", bufHold, strlen(bufHold));
	} else {
		error("ERROR: Could not read from file.");
	}
	return strlen(bufHold);
}

ssize_t netwrite(int fildes, const void *buf, size_t nbyte){
	
	write(sockfd, "w", 1);	
	char toWrite[256];
	strncpy(toWrite, (char*) buf, nbyte);
	toWrite[nbyte] = '\0';
	write(sockfd, toWrite, strlen(toWrite));
	char holdNum[10];
	read(sockfd, holdNum, strlen(holdNum));
	int written = atoi(holdNum);
	if(written >= 0) {
		printf("Number of bytes written: %i\n", written);
	} else {
		error("ERROR: Could not write to file.");
	}
	return written;
	
}

int netclose(int fd){
	write(sockfd, "c", 1);
	char closedMaybe[2];
	read(sockfd, closedMaybe, 2);
	int closed = atoi(closedMaybe);
	if(closed == 0) {
		printf("Successfully closed file.\n");
	} else {
		error("ERROR: Could not close file.");
	}
	fp = -1;
	return closed;
}
int main(int argc, char *argv[]) {
	if (argc < 3){
       		fprintf(stderr,"usage %s hostname port\n", argv[0]);
      		exit(0);
    	}
	
	portno = atoi(argv[2]);
    	serverIPAddress = gethostbyname(argv[1]);
    	if(serverIPAddress == NULL){
        	fprintf(stderr,"ERROR, no such host\n");
        	exit(0);
    	}
				
	// try to build a socket .. if it doesn't work, complain and exit
   	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    	if(sockfd < 0){
        	error("ERROR creating socket");
	}

    	bzero((char *) &serverAddressInfo, sizeof(serverAddressInfo));

   	serverAddressInfo.sin_family = AF_INET;
	serverAddressInfo.sin_port = htons(portno);
    	bcopy((char *)serverIPAddress->h_addr, (char *)&serverAddressInfo.sin_addr.s_addr, serverIPAddress->h_length);
    	if(connect(sockfd,(struct sockaddr *)&serverAddressInfo,sizeof(serverAddressInfo)) < 0){
        	error("ERROR connecting");
	}

	printf("Enter name of function:\n1) netopen: to open a file type o\n2) netclose: to close a file type c\n3) netread: to read from a file type r\n4) netwrite: to write into a file type w\n5) exit: 0\n");
	
	char str[256];
	scanf("%s", str);
	printf("\n");
		if(strlen(str) > 1) {
			error("Incorrect format\n Run again.\n");

		}
	helpSock(str[0]);
	while(str[0] != '0'){
		printf("Enter next command: ");
		scanf("%s", str);
		helpSock(str[0]);
		//printf("\n");

	}
	return 0;
}



