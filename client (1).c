/**
 * client.c
 *
 * @Author Matt Willis
 *
 * USD COMP 375: Computer Networks
 * Project 1
 *
 * DESCRIPTION: Program that creates socket connections with servers of domain name
 * esmarttech.com and requests/receives various types of weather condition
 * values to be printed back to the user.
 */

#define _XOPEN_SOURCE 600

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define BUFF_SIZE 1024
#define RQ_SIZE 1024

long prompt();
int connectToHost(char *hostname, char *port);
void mainLoop();
void connection_handler(void *rq_buff);

char buff[BUFF_SIZE]; // Array used to sending and receiving data from the server
char rq_buff[RQ_SIZE]; // Buffer for handling request responses

int main() {
	printf("WELCOME TO THE COMP 375 SENSOR NETWORK\n\n\n");
	mainLoop();
	return 0;
}

/**
 * Loop to keep asking user what they want to do and calling the appropriate
 * function to handle the selection.
 *
 */
void mainLoop() {
	while (1) {

		long selection = prompt();

		// Method for obtaining current time 
		time_t curtime;
		time(&curtime);

		switch (selection) {
			case 1: 
				// TODO: Handle case one by calling a function you write
				// Send air temperature command to connection_handler function
				// Print out current air temp stored in buff
				// Clear buffers
				
				strcat(rq_buff, "AIR TEMPERATURE");
				connection_handler(rq_buff);	
				printf("\nThe last AIR TEMPERATURE reading was %s, taken at %s\n", buff, ctime(&curtime));
				memset(buff, 0, BUFF_SIZE);
				memset(rq_buff, 0, RQ_SIZE);
				break;

			// TODO: add cases for other menu options
			case 2: 
				// Send relative humidity command to connection handler
				// function to interact with server
				// Print out current humidity and clear buffers
				
				strcat(rq_buff, "RELATIVE HUMIDITY");
				connection_handler(rq_buff);		
				printf("\nThe last RELATIVE HUMIDITY reading was %s, taken at %s\n", buff, ctime(&curtime));
				memset(buff, 0, BUFF_SIZE);
				memset(rq_buff, 0, RQ_SIZE);
				break;

			case 3:
				// Send wind speed command to connection handler function to
				// interact with server
				// Print out current wind speed and clear buffers

				strcat(rq_buff, "WIND SPEED");
				connection_handler(rq_buff);		
				printf("\nThe last WIND SPEED reading was %s, taken at %s\n", buff, ctime(&curtime));
				memset(buff, 0, BUFF_SIZE);
				memset(rq_buff, 0, RQ_SIZE);
				break;
								
			case 4:
				// Quit program
				printf("GOODBYE!\n");
				exit(0);	

			default:
				fprintf(stderr, "ERROR: Invalid selection\n");
				break;
		}
	}

}

/** 
 * Print command prompt to user and obtain user input.
 *
 * @return The user's desired selection, or -1 if invalid selection.
 */
long prompt() {
	// TODO: add printfs to print out the options
	
	printf("Which sensor would you like to read:\n\n");
	printf("\t(1) Air temperature\n");
	printf("\t(2) Relative humidity\n");
	printf("\t(3) Wind speed\n");
	printf("\t(4) Quit Program\n\n");
	printf("Selection: ");

	// Read in a value from standard input
	char input[10];
	memset(input, 0, 10); // set all characters in input to '\0' (i.e. nul)
	char *read_str = fgets(input, 10, stdin);

	// Check if EOF or an error, exiting the program in both cases.
	if (read_str == NULL) {
		if (feof(stdin)) {
			exit(0);
		}
		else if (ferror(stdin)) {
			perror("fgets");
			exit(1);
		}
	}

	// get rid of newline, if there is one
	char *new_line = strchr(input, '\n');
	if (new_line != NULL) new_line[0] = '\0';

	// convert string to a long int
	char *end;
	long selection = strtol(input, &end, 10);

	if (end == input || *end != '\0') {
		selection = -1;
	}

	return selection;
}

/**
 * Socket implementation of connecting to a host at a specific port.
 *
 * @param hostname The name of the host to connect to (e.g. "foo.sandiego.edu")
 * @param port The port number to connect to
 * @return File descriptor of new socket to use.
 */
int connectToHost(char *hostname, char *port) {
	// Step 1: fill in the address info in preparation for setting 
	//   up the socket
	int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;  // will point to the results

	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_INET;       // Use IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

	// get ready to connect
	if ((status = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}

	// Step 2: Make a call to socket
	int fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (fd == -1) {
		perror("socket");
		exit(1);
	}

	// Step 3: connect!
	if (connect(fd, servinfo->ai_addr, servinfo->ai_addrlen) != 0) {
		perror("connect");
		exit(1);
	}

	freeaddrinfo(servinfo); // free's the memory allocated by getaddrinfo

	return fd;
}


/**
 * Function to handle connecting to main.esmart, then sensor.esmart. Sends
 * request to sensor.esmart for user requested object and stores in buff to be
 * used in mainloop.
 *
 * @param *rq_buff  Buffer that contains the user's desired command for the
 * server
 */
void connection_handler(void *rq_buff) {

	int flags = 0;
	char tmp[RQ_SIZE]; // Temporary buffer to help with grabbing server responses
	
	// Connect to first host
	int server_fd = connectToHost("main.esmarttech.com", "47789");

	// Request to connect to main.esmart and check for errors
	strcat(buff, "AUTH password123\n");		
	if ((send(server_fd, buff, strlen(buff), 0)) <= 0) // ERROR CHECK
	{
		exit(1);
	}
	else
	{
		memset(buff, 0, BUFF_SIZE);
	}

	// Handle response from main.esmart
	// Upon successful connection, connect to sensor.esmart, otherwise restart
	if ((recv(server_fd, buff, BUFF_SIZE, flags)) <= 0) // ERROR CHECK
    {
		memset(buff, 0, BUFF_SIZE);
		exit(1);	
	}
	else
	{
		// Loop through server response to grab the password and port number
		// to connect to sensor.esmart 	        
		char *port;
		char *host;
		char *pass;
		char *tokens = strtok(buff, " ");
		int counter = 0;
		
		// Parses the server response into 3 sections by blank spaces(sensor.esmarttech.com,
		// port number to connect to, and password to connect to the server)
		while (tokens != NULL)
		{
			tokens =strtok(NULL, " ");
			if (counter == 0)
			{
				host = tokens;
			}
			if (counter == 1)
			{
				port = tokens;
			}
			if (counter == 2)
			{
				pass = tokens;
			}
			counter++;
		}
		
		// Close connection to main.esmart and connect to sensor.esmart
		close(server_fd);
		server_fd = connectToHost(host, port);
		
		strcpy(tmp, "AUTH ");
		strcat(tmp, pass);	
		if ((send(server_fd, tmp, strlen(tmp), 0)) <= 0) // ERROR CHECK
		{
			exit(1);
		}
	
       	else
       	{             
			memset(tmp, 0, RQ_SIZE);
			memset(buff, 0, BUFF_SIZE);
		}
	}
	if ((recv(server_fd, buff, BUFF_SIZE, 0)) <= 0) // ERROR CHECK
	{
		exit(1);
	}
	else
	{
		memset(buff, 0, BUFF_SIZE);
	}

	if ((send(server_fd, rq_buff, RQ_SIZE, 0)) <= 0) // ERROR CHECK
	{
		exit(1);
	}
	else
	{	
		memset(rq_buff, 0, RQ_SIZE);
	}
	if ((recv(server_fd, rq_buff, RQ_SIZE, 0)) <= 0) // ERROR CHECK
	{
		exit(1);
	}
	else
	{}
	
	// 305-326 Obtain the specific value the user requested from the server
	char *temp;
	char *type;
	char *tokens2 = strtok(rq_buff, " ");
	int counter2 = 0;
	while (tokens2 != NULL)
	{
		tokens2 =strtok(NULL, " ");
		if (counter2 == 0)
		{
			temp = tokens2;
		}
		if (counter2 == 1)
		{
			type = tokens2;
		}
		counter2++;
	}
	type = strtok(type, "\n");
	strcpy(buff, temp);
	strcat(buff, " ");
	strcat(buff, type);
	close(server_fd);
	return;	
}
