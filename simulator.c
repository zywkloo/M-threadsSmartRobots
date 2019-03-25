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
  memset(serverAddress, 0, sizeof(*serverAddress)); // zeros the struct
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
char canMoveTo(float newX, float newY, Environment* envPtr, int id) {
  int colFlag = 0; // not collided
  for (int i =0; i< envPtr->numRobots;i++){
    if(i != id){
      float xDist = envPtr->robots[i].x - envPtr->robots[id].x;
      float yDist = envPtr->robots[i].y - envPtr->robots[id].y;
      float distSquare = powf(xDist,2.0) + powf(yDist,2.0);
      colFlag = (distSquare > pow(2* ROBOT_RADIUS,2) )? 0:1;
    }
  }

  if ( colFlag == 1 ){  //if collided
    return NOT_OK_COLLIDE;
  } else if ( (newX+ROBOT_RADIUS) >ENV_SIZE || (newX - ROBOT_RADIUS) <0 ||  //if out of bound
              (newY - ROBOT_RADIUS) <0 ||  (newY - ROBOT_RADIUS)> ENV_SIZE ){
    return NOT_OK_BOUNDARY;
  }
  // delay to slow things down
  usleep(10000/(1 + envPtr->numRobots));
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
  unsigned int          addrSize;
  int                   bytesRcv;
  struct sockaddr_in    serverAddress,clientAddr;
  unsigned char         buffer[30],response[30];
  char*                 stopped = "ToBeStopped";
  Environment*          envPtr = e;

  // Initialize the server
  initializeServer( &serverSocket, &serverAddress);
  // Wait for clients now

  while (envPtr->shutDown == 0) {

    // ... Listening ... //
    addrSize = sizeof( clientAddr);
    clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &addrSize);
    if (clientSocket < 0) {
      printf("*** SERVER ERROR: Could accept incoming client connection.\n");
      exit(-1);
    }
    printf("SERVER: Received client connection.\n");

    // Go into infinite loop to talk to client
    while (1) {
      // Get the message from the client
      bytesRcv = recv(clientSocket, buffer, sizeof(buffer), 0);
      buffer[bytesRcv] = 0; // put a 0 at the end so we can display the string
      printf("SERVER: Received client request: %s\n", buffer);

        //handle STOP request
      if  (  buffer[0]== STOP ) {
        printf("SERVER: Sending \"%s\" to client\n", stopped);
        send(clientSocket, stopped, strlen(stopped), 0);
        break;
        // handle Robot client registration
      } else if (buffer[0]== REGISTER) {
        if (envPtr->numRobots >= MAX_ROBOTS ){
          // Respond with an "NOT OK" message
          // |  00   |
          // | NOTOK |
          response[0] =  NOT_OK;
          response[1] = 0;
          printf("SERVER: Reg - Sending \"%s\" to client\n", response);
          send(clientSocket, response, 1, 0);
          break;
        } else {
          printf("SERVER: new robot being created\n");
          //generate new robot
          unsigned int iniX = rand()%(ENV_SIZE -2* ROBOT_RADIUS) + ROBOT_RADIUS;  //generate ini x
          unsigned int iniY = rand()%(ENV_SIZE -2* ROBOT_RADIUS) + ROBOT_RADIUS;  //generate ini y
          int direction = rand()%360 -179;  //generate ini y
          Robot newRobot = {(float)iniX,(float)iniY, direction};
          envPtr->robots[envPtr->numRobots]= newRobot;

          printf("SERVER: new robot created at %d,%d.\n",iniX,iniY);
          // | 00 | 01 | 02 | 03 | 04 | 05 |    06     |     07        |
          // | OK | ID | Xh | Xl | Yh | Yl | Direction | DirectionSign |
          printf("        X,Y break down: %d,%d,%d,%d\n",(iniX / 256 ),(iniX % 256 ),(iniY / 256 ),(iniY % 256 ));
          // assemble the message
          response[0] =  OK;
          response[1] = (unsigned char ) (envPtr->numRobots);
          response[2] = (unsigned char ) (iniX / 256 );  //same effect with（ iniX & 0b1111111100000000）>> 8;
          response[3] = (unsigned char ) (iniX % 256 );
          response[4] = (unsigned char ) (iniY / 256 );
          response[5] = (unsigned char ) (iniY % 256 );
          response[6] = (unsigned char ) (abs(direction));
          response[7] = (unsigned char ) (direction >=0 ? 1 : 2);
          response[8] = 0;

          printf("SERVER: \"%s\" message assembled.\n",response);
          send(clientSocket, response, 9, 0);
          //send the response message
          printf("SERVER: Reg - Sending \"%s\" to client\n", response);
          printf("| 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 |\n");
          printf("| OK | %02d |%3d |%3d |%3d |%3d | %02d | %02d |\n",
                (unsigned int)(response[1]), //id
                (unsigned int)(response[2]),(unsigned int)(response[3]), //Xh, Xl
                (unsigned int)(response[4]),(unsigned int)(response[5]), //Yh, Yl
                (unsigned int)(response[6]),(unsigned int)(response[7]));
          envPtr->numRobots +=1;
          break;
        }
      } else if ( buffer[0]== MOVE_TO ) {  // handle Move
        // Get MOVE_TO request to server
        // | 00    | 01 | 02 | 03 | 04 | 05 |    06     |     07        |
        // |MOVE_TO| id | Xh | Xl | Yh | Yl | Direction | DirectionSign |
        int absDir = (unsigned int)(buffer[6] & 0b0000000011111111);
        int id =  (unsigned int)(buffer[1]);
        int sendX =  (int)(buffer[2]& 0b0000000011111111) * 256 + ( int )(buffer[3] & 0b0000000011111111);
        int sendY =  (int)(buffer[4]& 0b0000000011111111) * 256 + ( int )(buffer[5] & 0b0000000011111111);
        float moveX = ((float)sendX)/100;
        float moveY = ((float)sendY)/100;
        int dir = ((unsigned int) (buffer[7]) == 2 ) ?  (- absDir) : (absDir);
        printf("Normalize data \n");
        printf("| 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 |\n");
        printf("|MOVE| %02d |  %5f  |   %5f |  %5d  |\n",
               id,moveX,moveY,dir);
        int header =  canMoveTo (moveX,moveY,envPtr,id);
        if (header == OK){
          envPtr->robots[id].x = moveX;
          envPtr->robots[id].y = moveY;
          // send result response back to client
          // | 00    |
          // |  OK   |
          response[0] =  header;
          response[1] = 0;
          printf("SERVER: Move OK. \"%s\" move response sent.\n",response);
          send(clientSocket, response, 1, 0);
        } else if (header == NOT_OK_BOUNDARY){
          response[0] =  header;
          response[1] = 0;
          printf("SERVER: Move NO_OK_BOUNDARY. \"%s\" move response sent.\n",response);
          send(clientSocket, response, 1, 0);
        } else {
          response[0] =  header;
          response[1] = 0;
          printf("SERVER: Move NOT_OK_COLLIDE. \"%s\" move response sent.\n",response);
          send(clientSocket, response, 1, 0);
        }

        break; //handled this single request
      }
    }
    printf("SERVER: Closing client connection.\n");
    close(clientSocket); // Close this client's socket

    // If the client said to stop, then I'll stop myself
    if (buffer[0]== STOP ) {
      envPtr->shutDown = 1;
      break;
    }
  }
  // ... WRITE YOUR CLEANUP CODE HERE ... //
  pthread_exit(NULL);
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
  printf("SERVER: ptReqHdl created.\n");
  pthread_create(&ptRender, NULL, redraw, &environment);
  printf("SERVER: ptRender created.\n");

  pthread_join(ptReqHdl, NULL);
  printf("SERVER: ptReqHdl joined.\n");
  pthread_join(ptRender, NULL);
  printf("SERVER: ptRender joined.\n");
  
  // Wait for the threads to complete
  
  printf("SERVER: Shutting down.\n");

  return 0;
}
