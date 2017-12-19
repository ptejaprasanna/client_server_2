
#include <ctype.h>          /* for toupper */
#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset */
#include <sys/socket.h>     /* for socket, sendto, and recvfrom */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */

#define STRING_SIZE 1024

/* FUNCTION TO CHECK FOR EXISTING CLIENTS IN THE FILE */
int udp_duplicate_check(unsigned short);

/* STRUCTURE FOR SENDING THE MESSAGE */
struct udp_message_send{
  unsigned short client_secret_code;
  unsigned short server_secret_code;
  unsigned short length_of_name;
  char name[15];                    // FIXED LENGTH FOR MY SERVER NAME
};

/* STRUCTURE FOR RECEIVING THE MESSAGE */
struct udp_message_receive{
  unsigned short client_secret_code;
  unsigned short server_secret_code;
  unsigned short length_of_name;
  char name[80];
};

/* SERV_UDP_PORT is the port number on which the server listens for
   incoming messages from clients. You should change this to a different
   number to prevent conflicts with others in the class. */

#define SERV_UDP_PORT 48380

int main(void) {

   int sock_server;  /* Socket on which server listens to clients */

   struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
   unsigned short server_port;  /* Port number used by server (local port) */

   struct sockaddr_in client_addr;  /* Internet address structure that
                                        stores client address */
   /* MESSAGE STRUCTURES */
   struct udp_message_receive receivedUdpMessage;
   struct udp_message_send udpMessage;

   /* HARDCODING SERVER'S SECRET CODE */
   unsigned short h_server_secret_code = 14716;
   unsigned int client_addr_len;  /* Length of client address structure */
   int bytes_sent, bytes_recd; /* number of bytes sent or received */
   unsigned int i;  /* temporary loop variable */
   int flag;  // FLAG TO CHECK FOR ALREADY DISCOVERED CLIENTS
   FILE * cFile1; //creates a file if not the file does not exist
   cFile1 = fopen("ServerInfo", "a");
   fclose(cFile1);

   /* open a socket */

   if ((sock_server = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
      exit(1);
   }

   /* initialize server address information */

   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   server_addr.sin_addr.s_addr = htonl (INADDR_ANY);  /* This allows choice of
                                        any host interface, if more than one
                                        are present */
   server_port = SERV_UDP_PORT; /* Server will listen on this port */
   server_addr.sin_port = htons(server_port);

   /* bind the socket to the local server port */

   if (bind(sock_server, (struct sockaddr *) &server_addr,
                                    sizeof (server_addr)) < 0) {
      perror("Server: can't bind to local address\n");
      close(sock_server);
      exit(1);
   }

   /* wait for incoming messages in an indefinite loop */

   client_addr_len = sizeof (client_addr);

   for (;;) {
       
    /* INITIALIZING STRUCTURE VARIABLES TO ZERO */
      udpMessage.client_secret_code = 0;
      receivedUdpMessage.client_secret_code = 0;
      udpMessage.server_secret_code = 0;
      receivedUdpMessage.server_secret_code = 0;
      udpMessage.length_of_name = 0;
      receivedUdpMessage.length_of_name = 0;
      memset(udpMessage.name, 0, sizeof(udpMessage.name));
      memset(receivedUdpMessage.name, 0, sizeof(receivedUdpMessage.name));
       
   /* RECEIVE MESSAGE */
      bytes_recd = recvfrom(sock_server, &receivedUdpMessage, sizeof(struct udp_message_receive), 0,
                     (struct sockaddr *) &client_addr, &client_addr_len);
   
   /* CONVERTING DATA TO LITTLE ENDIAN */
      receivedUdpMessage.client_secret_code = ntohs(receivedUdpMessage.client_secret_code);
      unsigned short checker;
      checker = receivedUdpMessage.client_secret_code;              /*variable that will be passed to check for redundant port number*/
      receivedUdpMessage.server_secret_code = ntohs(receivedUdpMessage.server_secret_code);
      receivedUdpMessage.length_of_name = ntohs(receivedUdpMessage.length_of_name);

   /* COPYING NECESSARY DATA TO THE CHARACTER ARRAY TO LATER STORE IT IN THE FILE IF REQUIRED */
      char print_to_file[100];
   /* sprintf COPIES OUTPUT TO THE CHARACTER ARRAY WITHOUT DISPLAYING IT ON THE CONSOLE */
      sprintf(print_to_file, "%d,%s", receivedUdpMessage.client_secret_code, receivedUdpMessage.name );

   /* CHECK IF RECEIVED SECRET CODE MATCHES THE SERVER'S SECRET CODE */
      if((receivedUdpMessage.server_secret_code) == h_server_secret_code){

   /* ASSIGNING VARIABLES TO THE SEND MESSAGE STRUCTURE */
        udpMessage.client_secret_code = receivedUdpMessage.client_secret_code;
        udpMessage.server_secret_code = h_server_secret_code;
        memcpy(udpMessage.name, "teja-parlapalli", 80);
        udpMessage.length_of_name = strlen(udpMessage.name);

   /* CHECK THE FILE FOR ALREADY EXISTING CLIENTS */
        flag = udp_duplicate_check(checker);
        if(flag != checker){

   /* CREATING A POINTER TO THE FILE FOR DATA TO BE WRITTEN ONTO THE FILE */
          FILE * wFile;

   /* "a" -> APPEND MODE, NAME OF THE FILE IS ServerInfo */
          wFile = fopen ("ServerInfo", "a");
          
   /* WRITING TO THE FILE */
          fprintf(wFile, "%s\n", print_to_file);
          fclose(wFile);
        }
   /* CONVERTING DATA TO BIG ENDIAN */
        udpMessage.client_secret_code = htons(udpMessage.client_secret_code);
        udpMessage.server_secret_code = htons(udpMessage.server_secret_code);
        udpMessage.length_of_name = htons(udpMessage.length_of_name);

   /* SEND MESSAGE */
        bytes_sent = sendto(sock_server, &udpMessage, sizeof(struct udp_message_send) - 1, 0,
                 (struct sockaddr*) &client_addr, client_addr_len);
      }

   /* IF RECEIVED SECRET CODE DOES NOT MATCH THE SERVER'S SECRET CODE */
      else{
        udpMessage.client_secret_code = receivedUdpMessage.client_secret_code;
        udpMessage.server_secret_code = receivedUdpMessage.server_secret_code;
        udpMessage.length_of_name = 0;
        memset(udpMessage.name, 0, sizeof(udpMessage.name));
        //flag = 1;
        udpMessage.client_secret_code = htons(udpMessage.client_secret_code);
        udpMessage.server_secret_code = htons(udpMessage.server_secret_code);
        udpMessage.length_of_name = htons(udpMessage.length_of_name);

        bytes_sent = sendto(sock_server, &udpMessage, sizeof(struct udp_message_send) - 1 , 0,
                 (struct sockaddr*) &client_addr, client_addr_len);
      }
   }
}

/* FUNCTION TO CHECK FOR ALREADY EXISTING CLIENTS IN THE FILE */
int udp_duplicate_check(unsigned short checker){
  FILE * rFile;

/* "r" OPENS FILE IN READ MODE */
  rFile = fopen("ServerInfo", "r");
//  int c = fgetc(rFile);
  
 // if (c == EOF) {
  //  fclose(rFile);  
   // return 1;
    //exit(1);
 // }
  
  char buf[100];
  char temp[100];
  char text_client_code[6];
  unsigned short e_client_code;
  //unsigned short duplicate = 0;

  /* LOOP CHECKS EVERY LINE IN THE FILE */
  while (fgets(buf, sizeof(buf), rFile)) {
       sprintf(temp, "%s", buf);

   /* EXTRACTING SECRET CODE */
       int i;
       for(i = 0; i < strlen(temp); i++){
         sprintf(text_client_code + i, "%c", temp[i]);
           if(temp[i+1] == ','){
            break;
           }
       }
  /* CONVERT STRING TO INT */
       e_client_code = atoi (text_client_code);
       if(e_client_code == checker){
       fclose(rFile);
       return e_client_code;
       exit(1);      
       }
     }
   /* END OF EXTRACTION */
     fclose(rFile);
     return 0;
     exit(1);
}

