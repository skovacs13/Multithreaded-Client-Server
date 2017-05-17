#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>

void error(char *msg){
    perror(msg);
    exit(1);
}

void sendAndRec(void *badSocket){
	char path[256];
	
	char buffer[2];
	char bufHolder[256];
	int n;
	int socket = *((int*)badSocket);
	
	FILE *fp;
	char *token;
	while(1){
		/** If we're here, a client tried to connect **/
	
		// zero out the char buffer to receive a client message
	    	bzero(buffer,256);

		// try to read from the client socket
	    	n = read(socket,buffer,1);
		// if the read from the client blew up, complain and exit
	    	if (n < 0){
			error("ERROR reading from socket");
		}
		strcpy(bufHolder, buffer);
		
		if(strcmp(buffer, "o") == 0) {
			int f = read(socket,path,255);
			printf("PATH: %s\n", path);
			int i = 0;
			write(socket, "0", 1);
			read(socket, buffer, 2);
			while(path[i] >= 0 && path[i] <= 255 && path[i] != NULL){
				i++;
			}

			path[i] = '\0';
			char pathname[i];
			strncpy(pathname, path, i);
		
			if(strcmp(buffer, "0") == 0){
				fp = fopen(path, "r");
				printf("FP: %i\n", fp);
				if(fp == NULL){
					write(socket, "-1", 2);
					error("ERROR: File does not exist or read access is not given 1.\n");
				}
			}
			else if(strcmp(buffer, "1") == 0){
				
				fp = fopen(path, "w");
				printf("FP: %i\n", fp);
				if(fp == NULL){
					write(socket, "-1", 2);
					error("ERROR: File does not exist or read access is not given 2.\n");
				}
			}
			
			else if(strcmp(buffer, "2") == 0){
				printf("PATH: %s\n", path);
				fp = fopen(path, "a");
				printf("FP: %i\n", fp);
				if(fp == NULL){
					write(socket, "-1", 2);
					error("ERROR: File does not exist or read access is not given 3.\n");
				}
			}
		
			else{
				write(socket, "-1", 2);
					error("ERROR: File does not exist or read access is not given 4.\n");
			}
			sprintf(bufHolder, "%i", fileno(fp));
			
			write(socket, bufHolder, 2);
			
		}
		else if(strcmp(buffer, "r") == 0) {
			printf("Server: Read\n");

			char* val = malloc(10 * sizeof(char));
			read(socket, val, sizeof(val));
			int nbyte = atoi(val);

			
		
			char* result = malloc(nbyte * sizeof(char) + 1);
	
			
			
			//fgets(result, nbyte, fp);
			fseek(fp, 0, SEEK_SET);
			//printf("after %i\n", fp);
			fread(result, 1, nbyte, fp);
			printf("Result: %s, Len: %i, FP: %i\n", result, strlen(result), fp);
			//result[nbyte] = NULL; 
			//printf("after %s\n", result);

			if(strlen(result) <= 0){
				error("ERROR: Could not read file.\n");
			}
			write(socket, result, strlen(result));
			

		}
		else if(strcmp(bufHolder, "w") == 0) {

			char hold[256] = "";
			char buf[10];
			printf("Server: Write\n");
			read(socket, hold, 255);
			printf("Recieved from client: %s\n", hold);
			
			
			int written;
			char* writtenPnt = malloc(10 * sizeof(char));
			if(hold != NULL && strlen(hold) > 0){
				

				written = fwrite(hold , 1 , strlen(hold) , fp );
				printf("Finished writing: %s, FP: %i\n", hold, fp);
				sprintf(writtenPnt, "%d", written);
				write(socket, writtenPnt, strlen(writtenPnt));
				
			}
			else{	
				written = 0;
				sprintf(writtenPnt, "%d", written);
				write(socket, writtenPnt, strlen(writtenPnt));
				error("Error: Could not write specified bytes");
			}
			//free(writtenPnt);

		}
		else if(strcmp(buffer, "c") == 0) {
			int closed = fclose(fp);
			char closedPnt[5];
			sprintf(closedPnt, "%d", closed);
			write(socket, closedPnt, strlen(closedPnt));
			printf("File Closed\n");
			fp = 0;
			
		}
		
	}
	
	strncpy(path, token, strlen(token));
    	
	
	

	// try to write to the client socket
   	n = write(socket,"I got your message",18);
		
	// if the write to the client below up, complain and exit
    	if (n < 0) {
		error("ERROR writing to socket");
	}
}

int main(int argc, char *argv[]){
	//int sockfd, newsockfd, portno, clilen;
	//struct sockaddr_in serv_addr;
	//struct sockaddr_in cli_addr;
	
	int sockfd, newsockfd, portno, clilen;						// file descriptor for our server socket
											// utility variable - size of clientAddressInfo below
											// utility variable - for monitoring reading/writing from/to the socket
	pthread_t tid[10];								// char array to store data going to and coming from the socket

	struct sockaddr_in serverAddressInfo;					// Super-special secret C struct that holds address info for building our server socket
	struct sockaddr_in clientAddressInfo;					// Super-special secret C struct that holds address info about our client socket
	 
	// If the user didn't enter enough arguments, complain and exit
    	if (argc < 2){
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
   	}
	 
	/** If the user gave enough arguments, try to use them to get a port number and address **/

	// convert the text representation of the port number given by the user to an int  
	portno = atoi(argv[1]);
	 
	// try to build a socket .. if it doesn't work, complain and exit
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    	if (sockfd < 0) {
       		error("ERROR opening socket");
	}
	
	/** We now have the port to build our server socket on .. time to set up the address struct **/

	// zero out the socket address info struct .. always initialize!
	bzero((char *) &serverAddressInfo, sizeof(serverAddressInfo));

	// set the remote port .. translate from a 'normal' int to a super-special 'network-port-int'
	serverAddressInfo.sin_port = htons(portno);
	 
	// set a flag to indicate the type of network address we'll be using  
	serverAddressInfo.sin_family = AF_INET;
	
	// set a flag to indicate the type of network address we'll be willing to accept connections from
    	serverAddressInfo.sin_addr.s_addr = INADDR_ANY;
	 
	/** We have an address struct and a socket .. time to build up the server socket **/
     
    	// bind the server socket to a specific local port, so the client has a target to connect to      
    	if (bind(sockfd, (struct sockaddr *) &serverAddressInfo, sizeof(serverAddressInfo)) < 0) {
		error("ERROR on binding");
	}
			  
	// set up the server socket to listen for client connections
    	listen(sockfd,5);

	// determine the size of a clientAddressInfo struct
    	clilen = sizeof(clientAddressInfo);
	int i = 0;
	while(1){
		newsockfd = accept(sockfd, (struct sockaddr *) &clientAddressInfo, &clilen);
		if(newsockfd < 0){
			error("ERROR on accept.\n");
		}
		pthread_create(&tid[i], NULL, sendAndRec, &newsockfd);
		if(tid[i] < 0){
			error("ERROR creating thread.\n");
		}
		if(tid[i] == 0){
			close(sockfd);
			exit(0);
		}
		
		i++;
	}

	int j = 0;
	for(j = 0; j <= i; j++){
		pthread_join(tid[j], NULL);
	}
	
	return 0; 
}
