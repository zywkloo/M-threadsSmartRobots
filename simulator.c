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
// that may be handlesd are REGISTER and MOVE_TO.   Upon 
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

  // Initialize the server

  // Wait for clients now
  while (environment.shutDown == 0) {

    // ... WRITE YOUR CODE HERE ... //

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
  
  // Wait for the threads to complete
  
  printf("SERVER: Shutting down.\n");
}
