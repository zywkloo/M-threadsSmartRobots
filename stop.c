#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "simulator.h"


// Set up a client socket and connect to the server.  
// Return -1 if there was an error.
int connectToServer(int *sock,  struct sockaddr_in *address) {

    //int                 clientSocket;
    int                 status;
    // Create socket
    *sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (*sock < 0) {
        printf("*** CLIENT ERROR: Could open socket.\n");
        return -1;
        //exit(-1);

    }

    // Setup address
    memset(address, 0, sizeof(*address));
    address->sin_family = AF_INET;
    address->sin_addr.s_addr = inet_addr(SERVER_IP);
    address->sin_port = htons((unsigned short) SERVER_PORT);

    // Connect to server
    status = connect(*sock, (struct sockaddr *) address, sizeof(*address));
    if (status < 0) {
        printf("*** CLIENT ERROR: Could not connect.\n");
        return -1;
        //
    }
    return 1 ;
}


int main() {
  // ... ADD SOME VARIABLES  ... //
    int                 clientSocket;
    struct sockaddr_in  clientAddress;
    int                  bytesRcv;

    char                inStr[80];    // stores user input from keyboard
    char                buffer[80];   // stores sent and received data
  // Register with the server
    if (connectToServer(&clientSocket,  &clientAddress) < 0) {
        printf("S-CLIENT: connection failed.\n");
        exit(-1);
    }
    // Go into loop to commuincate with server now
    while (1) {
        // Get a command from the user
        printf("S-CLIENT: Enter command to send to server ... ");
        scanf("%s", inStr);

        // Send command string to server
        if ((strcmp(inStr,"stop") == 0) || (strcmp(inStr,"STOP") == 0)) {
            buffer[0]= STOP;
            buffer[1]= 0;
            printf("S-CLIENT: Sending \"%s\" to the server.\n",buffer);
            send(clientSocket, buffer, strlen(buffer), 0);

            // Get response from server, should be "OK"
            bytesRcv = recv(clientSocket, buffer, 80, 0);
            buffer[bytesRcv] = 0; // put a 0 at the end so we can display the string
            printf("S-CLIENT: Got back response \"%s\" from server.\n", buffer);
        }

        if ((strcmp(inStr,"done") == 0) || (strcmp(inStr,"stop") == 0) || (strcmp(inStr,"STOP") == 0))
            break;
    }

    close(clientSocket);  // Don't forget to close the socket !
    printf("S-CLIENT: Shutting down.\n");


}
