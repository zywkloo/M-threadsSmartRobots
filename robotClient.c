#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "simulator.h"


// Set up a client socket and connect to the server.  
// Return -1 if there was an error.
int connectToServer(int *sock,  struct sockaddr_in *address) {
  // ... WRITE YOUR CODE HERE ... //

}




int main() {
  // ... ADD SOME VARIABLE HERE ... //
  
  // Set up the random seed
  srand(time(NULL));
  
  // Register with the server

  // Send register command to server.  Get back response data
  // and store it.   If denied registration, then quit.
  
  // Go into an infinite loop exhibiting the robot behavior
  while (1) {
    // Connect to the server
    
    // Compute a forward location and check if it is ok
        
    // Send MOVE_TO request to server
    
    // Get response from server.

    // If response is "OK" then move forward

    // Otherwise, we could not move forward, so make a turn.
    // If we were turning from the last time we collided, keep
    // turning in the same direction, otherwise choose a random 
    // direction to start turning.
    
    // Uncomment line below to slow things down a bit 
    // usleep(1000);
  }
}
