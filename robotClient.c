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
    printf("R-CLIENT ERROR: Could not connect.\n");
    return -1;
    //
  }
  return 1 ;

}


int main() {
  // ... ADD SOME VARIABLE HERE ... //
  int                 clientSocket;
  struct sockaddr_in  clientAddress;
  int                  bytesRcv;
  char                 buffer[80];   // stores sent and received data
  // | 00 | 01 | 02 | 03 | 04 | 05 |    06     |     07        |
  // | OK | id | Xh | Xl | Yh | Yl | Direction | DirectionSign |
  unsigned char header;
  int id;
  unsigned int curX;
  unsigned int curY;
  int dir;
  /**
  unsigned int testInt = 198;
  unsigned char test= (unsigned char)testInt;
  printf("testInt %d,testInt %c ,convert %d ..",testInt ,test ,(int)test);
   **/

  // Set up the random seed
  srand(time(NULL));

  // Register with the server
  if (connectToServer(&clientSocket,  &clientAddress) < 0) {
    printf("R-CLIENT: connection failed.\n");
    exit(-1);
  }

  // Send register command to server.  Get back response data
  // and store it.   If denied registration, then quit.
  header = REGISTER;
  buffer[0]= header;
  buffer[1]= '\0';
  printf("R-CLIENT: Now Sending status %d \"%s\" to the server.\n",REGISTER,buffer);
  send(clientSocket, buffer, strlen(buffer), 0);

  // Get response from server
  bytesRcv = recv(clientSocket, buffer, 80, 0);

  if ( buffer[0]== NOT_OK  ){
    buffer[bytesRcv] = 0; // put a 0 at the end so we can display the string
    printf("R-CLIENT: Robot Exceeds MaxNum. Unable to register.\n");
    exit(-1);
  } else if (buffer[0]== OK ){  // may contain \0 before the last letter, so don't check len
    buffer[8] = 0; // put a 0 at the end so we can display the string
    printf("R-CLIENT: Got back \"");
    for (int i=0;i< 8;i++ ) printf(" %c",buffer[i]);
    printf("\" from server.\n");
    // | 00 | 01 | 02 | 03 | 04 | 05 |    06     |     07        |
    // | OK | id | Xh | Xl | Yh | Yl | Direction | DirectionSign |
    printf("| 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 |\n");
    printf("| OK | %02u |%3u |%3u |%3u |%3u | %3u | %2u |\n",
           (unsigned int)(buffer[1]), //id
           (unsigned int)(buffer[2]),( int )(buffer[3] & 0b0000000011111111), //Xh, Xl
           (unsigned int)(buffer[4]),( int )(buffer[5] & 0b0000000011111111), //Yh, Yl
           (unsigned int)(buffer[6] & 0b0000000011111111),(unsigned int)(buffer[7]));
    int absDir = (unsigned int)(buffer[6] & 0b0000000011111111);
    id =  (unsigned int)(buffer[1]);
    curX =  (unsigned int)(buffer[2]) * 256 + ( int )(buffer[3] & 0b0000000011111111);
    curY =  (unsigned int)(buffer[4]) * 256 + ( int )(buffer[5] & 0b0000000011111111);
    dir = ((unsigned int) (buffer[7]) == 2 ) ?  (- absDir) : (absDir);
    printf("Normalize data \n");
    printf("| 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 |\n");
    printf("| OK | %02d |  %3d    |   %3d   |  %5d  |\n",
           id,curX,curY,dir);
  } else {
    printf("R-CLIENT: Err - Received \"%s\" buffer.len \"%lu\".  Data corrupted.\n",buffer,strlen(buffer));
    exit(-1);
  }
  
  // Go into an infinite loop exhibiting the robot behavior
  while (1) {
    // Connect to the server
    if (connectToServer(&clientSocket,  &clientAddress) < 0) {
      printf("R-CLIENT: connection failed.\n");
      exit(-1);
    }
    // Compute a forward location and check if it is ok
    float newX = curX + ROBOT_SPEED*cos(dir);
    float newY = curY + ROBOT_SPEED*sin(dir);
    // Send MOVE_TO request to server
    // | 00    | 01 | 02 | 03 | 04 | 05 |    06     |     07        |
    // |MOVE_TO| id | Xh | Xl | Yh | Yl | Direction | DirectionSign |
    header = MOVE_TO;
    buffer[0]= header;
    buffer[1]= (unsigned char ) id;
    buffer[2]= id;
    buffer[3]= id;
    buffer[4]= id;
    buffer[5]= id;
    buffer[6]= (unsigned char ) absDir ;
    buffer[7]= (unsigned char ) (direction >=0 ? 1 : 2);
    buffer[8]= 0;
    printf("R-CLIENT: Now Sending status %d \"%s\" to the server.\n",MOVE_TO,buffer);
    send(clientSocket, buffer, 9, 0);
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
