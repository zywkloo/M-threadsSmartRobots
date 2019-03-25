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
  }
  return 1 ;
}


int main() {
  // ... ADD SOME VARIABLE HERE ... //
  int                  clientSocket;
  struct sockaddr_in   clientAddress;
  int                  bytesRcv;
  char                 buffer[80];   // stores sent and received data
  // | 00 | 01 | 02 | 03 | 04 | 05 |    06     |     07        |
  // | OK | id | Xh | Xl | Yh | Yl | Direction | DirectionSign |
  unsigned char header;
  int id;
  float curX;
  float curY;
  int dir, absDir;
  int rotation = -1;  //-1 = not decided, 1 = clockwise , 0 = counter clockwise.
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
    //location init;
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
    // location init: get data from server , and then assign to client data;
    absDir = (unsigned int)(buffer[6] & 0b0000000011111111);
    id =  (unsigned int)(buffer[1]);
    curX =  (int)(buffer[2]& 0b0000000011111111) * 256 + ( int )(buffer[3] & 0b0000000011111111);
    curY =  (int)(buffer[4]& 0b0000000011111111) * 256 + ( int )(buffer[5] & 0b0000000011111111);
    dir = ((unsigned int) (buffer[7]) == 2 ) ?  (- absDir) : (absDir);
    printf("Normalize data \n");
    printf("| 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 |\n");
    printf("| OK | %02d |  %5.2f    |   %5.2f   |  %5d  |\n",
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
      break;
    }
    // Compute a forward location and check if it is ok
    float newX = curX + ROBOT_SPEED*cos(dir);
    float newY = curY + ROBOT_SPEED*sin(dir);
    int sendX = (int) (newX * 100);  //amplify the float X coordinates by 100 times.Keep them Int.
    int sendY = (int) (newY * 100);  //amplify the float Y coordinates by 100 times.Keep them Int.
    // Send MOVE_TO request to server
    // | 00    | 01 | 02 | 03 | 04 | 05 |    06     |     07        |
    // |MOVE_TO| id | Xh | Xl | Yh | Yl | Direction | DirectionSign |
    header = MOVE_TO;
    buffer[0]= header;
    buffer[1]= (unsigned char ) id;
    buffer[2]= (unsigned char ) (sendX / 256 );  //same effect with（ sendX & 0b1111111100000000）>> 8;;
    buffer[3]= (unsigned char ) (sendX % 256 ) ;
    buffer[4]= (unsigned char ) (sendY / 256 );
    buffer[5]= (unsigned char ) (sendY % 256 );;
    buffer[6]= (unsigned char ) absDir ;    //absolute value of direction
    buffer[7]= (unsigned char ) (dir >=0 ? 1 : 2);//2 means - ; 1 means +
    buffer[8]= 0;
    printf("R-CLIENT: MOVE - Sending status %d \"%s\" to the server.\n",MOVE_TO,buffer);
    printf("        : newX= %f, newY=%f .\n",newX,newY);
    send(clientSocket, buffer, 9, 0);
    //send the response message
    printf("|  00   | 01 | 02 | 03 | 04 | 05 | 06 | 07 |\n");
    printf("|MOVE_TO| %02d |%3d |%3d |%3d |%3d | %02d | %02d |\n",
           (unsigned int)(buffer[1]), //id
           (unsigned int)(buffer[2]),(unsigned int)(buffer[3]), //Xh, Xl
           (unsigned int)(buffer[4]),(unsigned int)(buffer[5]), //Yh, Yl
           (unsigned int)(buffer[6]),(unsigned int)(buffer[7]));
    // Get response from server.
    bytesRcv = recv(clientSocket, buffer, 80, 0);
    // If response is "OK" then move forward
    if (buffer[0]== OK){
      printf("R-CLIENT: Receive MOVE - Response OK from the server.\n");
      curX = newX;
      curY = newY;
      rotation = -1;
      // Otherwise, we could not move forward, so make a turn.
      // If we were turning from the last time we collided, keep
      // turning in the same direction, otherwise choose a random
      // direction to start turning.
    } else { //buffer[0]== NOT_OK_COLLIDE or NOT_OK_BOUNDARY
      printf("R-CLIENT: Receive MOVE - Response NOT_OK from the server.\n");
      if (rotation == -1){
        rotation = rand()%2;
      }
      if (rotation == 1){
        dir +=  ROBOT_TURN_ANGLE;
        if (dir > 180) dir -= 360;
      } else {
        dir -=  ROBOT_TURN_ANGLE;
        if (dir < -179) dir += 360;
      }
    }
    close(clientSocket);  // Don't forget to close the socket !
    // Uncomment line below to slow things down a bit 
     usleep(100000);
  }
}
