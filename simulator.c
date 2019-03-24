#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "simulator.h"

#include "display.c"


// The environment that contains all the robots
Environment    environment;  


// Initialize the server by creating the server socket, 
// setting up the server address, binding the server socket 
// and setting up the server to listen for the MAX_ROBOTS 
// number of robot clients
void initializeServer(int *serverSocket, 
                      struct sockaddr_in  *serverAddress) {
  // ... WRITE SOME CODE HERE ... //
  int status;
  // Create the server socket
  *serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (*serverSocket < 0) {
    printf("*** SERVER ERROR: Could not open socket.\n");
    exit(-1);
  }

  // Setup the server address
  memset(&serverAddress, 0, sizeof(serverAddress)); // zeros the struct
  serverAddress->sin_family = AF_INET;
  serverAddress->sin_addr.s_addr = htonl(INADDR_ANY);
  serverAddress->sin_port = htons((unsigned short) SERVER_PORT);

  // Bind the server socket
  status = bind(*serverSocket,  (struct sockaddr *)serverAddress, sizeof(*serverAddress));
  if (status < 0) {
    printf("*** SERVER ERROR: Could not bind socket.\n");
    exit(-1);
  }

  // Set up the line-up to handle up to 5 clients in line
  status = listen(*serverSocket, MAX_ROBOTS);
  if (status < 0) {
    printf("*** SERVER ERROR: Could not listen on socket.\n");
    exit(-1);
  }

}


// Determine whether the registered robot with id i is able 
// to move to the location specified by (newX, newY). Return 
// OK if it is able.  Return NOT_OK_COLLIDE if moving to that 
// location would cause the robot to collide with another robot.
// Return NOT_OK_BOUNDARY if moving to that location would cause 
// the robot to go beyond the environmental boundary.
char canMoveTo(/* PUT PARAMS HERE */) {


  // ... WRITE YOUR CODE HERE ... //


  // delay to slow things down
  usleep(10000/(1 + environment.numRobots)); 
  return OK;
}



// Handle client requests coming in through the server socket.  
// This code should run indefinitiely.  It should wait for a 
// client to send a request, process it, and then close the 
// client connection and wait for another client.  The requests 
// that may be handled are REGISTER and MOVE_TO.   Upon
// receiving a REGISTER request, the server should choose a 
// random location and direction within the environment (which 
// MUST be within the boundaries and must NOT be overlapping 
// another robot).  Upon receiving a MOVE_TO request, the 
// code should send a single byte to the client which is either 
// OK or NOT_OK_BOUNDARY (if the robot would collide with the 
// boundary at the requested location) or NOT_OK_COLLIDE (if the 
// robot would collide with another robot at the requested 
// location).  Parameter *e is a pointer to the environment.
void *handleIncomingRequests(void *e) {
  // ... ADD SOME VARIABLE HERE... //
  int                   serverSocket,clientSocket;
  int                   addrSize, bytesRcv;
  int                   connectedNum = 0;
  struct sockaddr_in    serverAddress,clientAddr;
  char                  buffer[30];
  char*                 response = "OK";
  Environment*          envPtr = (Environment *)e;
  // Initialize the server
  initializeServer( &serverSocket, &serverAddress);
  // Wait for clients now
  while (envPtr->shutDown == 0) {

    // ... WRITE YOUR CODE HERE ... //
    addrSize = sizeof( clientAddr);
    clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &addrSize);
    if (clientSocket < 0) {
      printf("*** SERVER ERROR: Could accept incoming client connection.\n");
      exit(-1);
    }
    printf("SERVER: Received client connection.\n");
    connectedNum++;

    // Go into infinite loop to talk to client
    while (1) {
      // Get the message from the client
      bytesRcv = recv(clientSocket, buffer, sizeof(buffer), 0);
      buffer[bytesRcv] = 0; // put a 0 at the end so we can display the string
      printf("SERVER: Received client request: %s\n", buffer);

      // Respond with an "OK" message
      printf("SERVER: Sending \"%s\" to client\n", response);
      send(clientSocket, response, strlen(response), 0);
      if  (strcmp(buffer,"2") == 0) {
        break;
      }
    }
    printf("SERVER: Closing client connection.\n");
    close(clientSocket); // Close this client's socket

    // If the client said to stop, then I'll stop myself
    if (strcmp(buffer,"2") == 0)
      break;
  }

  // ... WRITE YOUR CLEANUP CODE HERE ... //
}



int main() {
  // So far, the environment is NOT shut down
  environment.shutDown = 0;
  
  // Set up the random seed
  srand(time(NULL));

  // Spawn threads to handle incoming requests and
  // update the display
  pthread_t ptReqHdl,ptRender;

  pthread_create(&ptReqHdl, NULL, handleIncomingRequests, &environment);
  pthread_create(&ptReqHdl, NULL, redraw, &environment);

  printf("SERVER: ptReqHdl and ptReqHdl created");

  pthread_join(ptReqHdl, NULL);
  pthread_join(ptRender, NULL);

  printf("SERVER: ptReqHdl and ptReqHdl joined");
  
  // Wait for the threads to complete
  
  printf("SERVER: Shutting down.\n");
}
