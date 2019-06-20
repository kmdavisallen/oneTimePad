/************************************************************
**Author: Kevin Allen
**Date 8/11/18
**Description:  CS344 program 4: otp_dec_d
************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int childExitStatus = 0;
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	char buffer[256];
	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2) { fprintf(stderr, "USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");
	
	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");

	int* numConnects;
	*numConnects = 0;
	while (1) {
		listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections
		while (*numConnects <= 5) {
			*numConnects++;
			// Accept a connection, blocking if one is not available until one connects
			sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
			establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
			if (establishedConnectionFD < 0) error("ERROR on accept");

			int spawnid = fork();
			if (spawnid == -1) {
				fprintf(stderr, "fork error");
				exit(1);
			}
			else if (spawnid == 0) {
				// Get the password from the client
				charsRead = 0;
				memset(buffer, '\0', 256);
				charsRead = recv(establishedConnectionFD, buffer, 255, 0); // Read the client's message from the socket
				if (charsRead < 0) error("ERROR reading from socket");

				if (strcmp(buffer, "Surely you can't be serious") != 0) {
					send(establishedConnectionFD, "Wrong Password", 15, 0);
					fprintf(stderr, "Wrong password\n");
					exit(2);
				}

				// Send a code message back to the client
				charsRead = send(establishedConnectionFD, "I am serious, and don't call me Shirley", 40, 0); // Send success back
				if (charsRead < 0) error("ERROR writing to socket");

				//get size of message
				memset(buffer, '\0', 256);
				charsRead = recv(establishedConnectionFD, buffer, 255, 0); // Read the client's message from the socket
				if (charsRead < 0) error("ERROR reading from socket");

				//get message
				int size = atoi(buffer);
				char* message = malloc((size) * sizeof(char));
				memset(message, '\0', sizeof(message));
				char* cypher = malloc((size) * sizeof(char));
				memset(cypher, '\0', sizeof(cypher));

				//dummy send to coordinate recv/sends
				send(establishedConnectionFD, "you would think there's an easier way", 38, 0);

				charsRead = 0;
				while (charsRead < (size - 1)) {
					charsRead = charsRead + recv(establishedConnectionFD, buffer, 255, 0);
					strcat(message, buffer);
				}

				//dummy send to coordinate recv/sends
				send(establishedConnectionFD, "but apparently there isn't", 27, 0);

				//get key
				char* key = malloc(size * sizeof(char));
				charsRead = 0;
				while (charsRead < (size - 1)) {
					charsRead = charsRead + recv(establishedConnectionFD, buffer, 255, 0);
					strcat(key, buffer);
				}

				char cypherAlpha[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";		//for indexing codes
				int i, j, msgNum, keyNum = 0;
				for (i = 0; i < size - 1; i++) {
					for (j = 0; j < 27; j++) {
						if (message[i] == cypherAlpha[j]) {
							msgNum = j;
						}
						if (key[i] == cypherAlpha[j]) {
							keyNum = j;
						}
					}

					if ((msgNum - keyNum) < 0) {
						cypher[i] = cypherAlpha[(msgNum - keyNum) + 27];
					}
					else {
						cypher[i] = cypherAlpha[(msgNum - keyNum)];
					}
				}

				send(establishedConnectionFD, cypher, size, 0);

				close(establishedConnectionFD); // Close the existing socket which is connected to the client
				
				exit(0);
			}
			else {
				int childpid = waitpid(-1, &childExitStatus, WNOHANG);
				if (childpid > 0) {
					*numConnects--;
				}
			}
		}
	}
	close(listenSocketFD); // Close the listening socket
	return 0;
}