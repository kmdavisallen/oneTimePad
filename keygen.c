/************************************************************
**Author: Kevin Allen
**Date 8/11/18
**Description:  CS344 program 4: Keygen
************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char* argv[]) {
	if (argc < 2) {		//make sure user supplied size of key to generate
		fprintf(stderr, "Insufficent arguments\n");
		exit(1);
	}
	int numKey = atoi(argv[1]);	//convert to integer argument
	srand(time(NULL));

	int i = 0;
	for (i = 0; i < numKey; i++) {		//adapted from https://stackoverflow.com/questions/19724346/generate-random-characters-in-c
		char key = "ABCDEFGHIJKLMNOPQRSTUVWXYZ "[rand() % 27]; //pick random index into alphabet array 
		printf("%c", key);
	}
	
	printf("\n");
	return 0;
}