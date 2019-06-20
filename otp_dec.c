/************************************************************
**Author: Kevin Allen
**Date 8/11/18
**Description:  CS344 program 4: otp_enc
************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <fcntl.h>

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[256];

	if (argc < 4) { fprintf(stderr, "USAGE: %s hostname port\n", argv[0]); exit(0); } // Check usage & args

																					  // Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

																											// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

	// Send message to server to confirm correct server connection
	charsWritten = 0;
	char password[] = "Surely you can't be serious";
	while (charsWritten < strlen(password)) {
		charsWritten = charsWritten + send(socketFD, password, strlen(password), 0); // Write to the server
		if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	}

	//get confirmation that connected to opt_enc_d
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");

	if (strcmp(buffer, "I am serious, and don't call me Shirley") != 0) {
		fprintf(stderr, "cannot use otp_dec with otp_enc_d\n");
		exit(2);
	}

	//get messge from file
	int size = 0;
	int fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "error, file not found\n");
		exit(1);
	}
	else {	//get size of file 
		lseek(fd, 0, SEEK_SET);		//make sure file pointer is at beginning of file
		size = lseek(fd, 0, SEEK_END);
		lseek(fd, 0, SEEK_SET);
	}
	char* message = malloc(size * sizeof(char));
	read(fd, message, size);
	message[strcspn(message, "\n")] = '\0';

	int i = 0;
	for (i = 0; i < size; i++) {	//loop through message to check for invalid characters
		if ((message[i] < 'A' || message[i] > 'Z') && message[i] != ' ' && message[i] != '\0' && message[i] != '\n') {
			fprintf(stderr, "invalid character found\n");
			exit(1);
		}
	}
	close(fd);

	//get key from file
	int keySize = 0;
	int keyFd = open(argv[2], O_RDONLY);
	if (keyFd < 0) {
		fprintf(stderr, "error, file not found\n");
		exit(1);
	}
	else {	//get size of file 
		lseek(keyFd, 0, SEEK_SET);		//make sure file pointer is at beginning of file
		keySize = lseek(keyFd, 0, SEEK_END);
		lseek(keyFd, 0, SEEK_SET);
	}

	if (keySize < size) {
		fprintf(stderr, "Key is incompatable size\n");
		exit(1);
	}

	char* key = malloc(keySize * sizeof(char));
	read(keyFd, key, keySize);
	char* trunkKey = malloc(size * sizeof(char));
	strncpy(trunkKey, key, size);

	for (i = 0; i < keySize; i++) {	//loop through message to check for invalid characters
		if ((key[i] < 'A' || key[i] > 'Z') && key[i] != ' ' && key[i] != '\0' && key[i] != '\n') {
			fprintf(stderr, "invalid character found\n");
			exit(1);
		}
	}
	close(keyFd);
	free(key);

	//send size of message to server
	char sizeC[20];
	sprintf(sizeC, "%d", size);
	send(socketFD, sizeC, strlen(sizeC), 0);

	//dummy recieved for timing
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end

	 // Send message to server
	charsWritten = 0;
	charsWritten = send(socketFD, message, strlen(message), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(message)) printf("CLIENT: WARNING: Not all data written to socket!\n");

	//dummy recieved for timing
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end

	//send key to server
	charsWritten = 0;
	charsWritten = charsWritten + send(socketFD, trunkKey, strlen(trunkKey), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(trunkKey)) printf("CLIENT: WARNING: Not all data written to socket!\n");

	memset(message, '\0', size); // Clear out the buffer again for reuse
	charsRead = 0;
	while (charsRead < (size - 1)) {
		memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
		charsRead = charsRead + recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
		strcat(message, buffer);
	}

	fprintf(stdout, "%s\n", message);
	close(socketFD); // Close the socket

	free(message);
	
	return 0;
}
