#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h> //internet address library
#include <netdb.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <sys/stat.h>
#include <arpa/inet.h>

#include <openssl/sha.h> //hashing pieces

#include "bt_lib.h"
#include "bt_setup.h"
#include "bt_parser.h"

/**
 * calc_id(char * ip, unsigned short port, char *id) -> void
 *
 * Calculates the peer id based on ip and port number
 *
 **/

void calc_id(char * ip, unsigned short port, char *id){
  char data[256];
  int len;
 
  //format print
  len = snprintf(data,256,"%s%u",ip,port);

  //id is just the SHA1 of the ip and port string
  SHA1((unsigned char *) data, len, (unsigned char *) id); 

  return;
}


/**
 * init_peer(peer_t * peer, int id, char * ip, unsigned short port) -> int
 *
 *
 * initialize the peer_t structure peer with an id, ip address, and a
 * port. Further, it will set up the sockaddr such that a socket
 * connection can be more easily established.
 *
 * Return: 0 on success, negative values on failure. Will exit on bad
 * ip address.
 *   
 **/
int init_peer(peer_t *peer, char * id, char * ip, unsigned short port){
    
  struct hostent * hostinfo;
  //set the host id and port for referece
  memcpy(peer->id, id, ID_SIZE);
  peer->port = port;
    
  //get the host by name
  if((hostinfo = gethostbyname(ip)) ==  NULL){
    perror("gethostbyname failure, no such host?");
    herror("gethostbyname");
    exit(1);
  }
  
  //zero out the sock address
  bzero(&(peer->sockaddr), sizeof(peer->sockaddr));
      
  //set the family to AF_INET, i.e., Iternet Addressing
  peer->sockaddr.sin_family = AF_INET;
    
  //copy the address to the right place
  bcopy((char *) (hostinfo->h_addr), 
        (char *) &(peer->sockaddr.sin_addr.s_addr),
        hostinfo->h_length);
    
  //encode the port
  peer->sockaddr.sin_port = htons(port);
  
  return 0;

}

/**
 * print_peer(peer_t *peer) -> void
 *
 * print out debug info of a peer
 *
 **/
void print_peer(peer_t *peer){
  int i;

  if(peer){
    printf("peer: %s:%u ",
           inet_ntoa(peer->sockaddr.sin_addr),
           peer->port);
    printf("id: ");
    for(i=0;i<ID_SIZE;i++){
      printf("%02x",peer->id[i]);
    }
    printf("\n");
  }
}

/**
 * getDigits(int number ) -> int
 *
 * Returns the number of digits in a number
 *
 **/
 
int getDigits(int number ){
  int length = 1;
  
  while (number /= 10)
    length ++;

  return length;
}

/**
 * getDataFromSockMsg(char * sockMsg, char * sockData, unsigned  int length)  ->  void
 *
 * Retrieves the actual data from the socket message
 *
 **/

void getDataFromSockMsg(char * sockMsg, char * sockData, unsigned int length) {
  unsigned int offset;
  
  for (offset = 2; offset < length; offset ++){
    if (sockMsg[offset] == '+'){
	  offset ++;
	  break;
    }
  }
  strcpy(sockData, sockMsg + offset);
}

/**
 * getLengthFromSockMsg(char * sockMsg) -> int
 *
 * Retrieves the length of actual data from the socket message
 *
 **/

int getLengthFromSockMsg(char * sockMsg) {
  int offset, j;
  char lngth[30];

  for(offset = 2, j = 0; sockMsg[offset] != '+' ; offset++, j++)
    lngth[j] = sockMsg[offset];

  lngth[j] = '\0';
  offset = atoi(lngth);

  return offset;
}

/**
 * printProgress(char * bitfield) -> void
 *
 * Retrieves the length of actual data from the socket message
 *
 **/
 
void printProgress(char * fileName, char * bitfield) {
  unsigned int num_of_pieces = 0;

  for(unsigned int i = 0; i < strlen(bitfield); i++)
    if(bitfield[i] == '1')
      num_of_pieces++;

  double dwnPercnt = (double)num_of_pieces/strlen(bitfield);
  printf("\n******   File:%s    Download Progress: %.2f%%   ****** \n\n", fileName, dwnPercnt * 100);
  fflush(stdout);
}