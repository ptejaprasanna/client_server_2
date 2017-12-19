
#include <ctype.h>          /* for toupper */
#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset */
#include <sys/socket.h>     /* for socket, bind, listen, accept */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */

/* STRUCTURE FOR THE TCP MESSAGE */
struct tcp_message{
  unsigned short client_secret_code;
  unsigned short server_secret_code;
  unsigned short length_of_name;
};

#define STRING_SIZE 1024

/* SERV_TCP_PORT is the port number on which the server listens for
   incoming requests from clients. You should change this to a different
   number to prevent conflicts with others in the class. */

#define SERV_TCP_PORT 48380

int main(void) {

   int sock_server_tcp;  /* Socket on which server listens to clients */
   int sock_connection;  /* Socket on which server exchanges data with client */

   struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
   unsigned int server_addr_len;  /* Length of server address structure */
   unsigned short server_port;  /* Port number used by server (local port) */

   /* DECLARING VARIABLES FOR STRUCT tcp_message */
   struct tcp_message tcpMessage, receivedTcpMessage;

   struct sockaddr_in client_addr;  /* Internet address structure that
                                        stores client address */
   unsigned int client_addr_len;  /* Length of client address structure */
   int bytes_sent, bytes_recd; /* number of bytes sent or received */
   unsigned int i;  /* temporary loop variable */
   //unsigned secret_server_code = 14716;
   //string server_name = "teja-parlapalli";

   /* open a socket */

   if ((sock_server_tcp = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
      exit(1);
   }

   /* initialize server address information */

   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   server_addr.sin_addr.s_addr = htonl (INADDR_ANY);  /* This allows choice of
                                        any host interface, if more than one
                                        are present */
   server_port = SERV_TCP_PORT; /* Server will listen on this port */
   server_addr.sin_port = htons(server_port);

   /* bind the socket to the local server port */

   if (bind(sock_server_tcp, (struct sockaddr *) &server_addr,
                                    sizeof (server_addr)) < 0) {
      close(sock_server_tcp);
      exit(1);
   }

   /* listen for incoming requests from clients */

   if (listen(sock_server_tcp, 50) < 0) {    /* 50 is the max number of pending */
      close(sock_server_tcp);
      exit(1);
   }

   client_addr_len = sizeof (client_addr);

   /* wait for incoming connection requests in an indefinite loop */

   for (;;) {
      
      /* INITIALIZING STRUCTURE VARIABLES TO ZERO */
      tcpMessage.client_secret_code = 0;
      receivedTcpMessage.client_secret_code = 0;
      tcpMessage.server_secret_code = 0;
      receivedTcpMessage.server_secret_code = 0;
      tcpMessage.length_of_name = 0;
      receivedTcpMessage.length_of_name = 0;


      sock_connection = accept(sock_server_tcp, (struct sockaddr *) &client_addr,
                                         &client_addr_len);
                     /* The accept function blocks the server until a
                        connection request comes from a client */
      if (sock_connection < 0) {
         close(sock_server_tcp);
         exit(1);
      }

      /* receive the message */

      bytes_recd = recv(sock_connection, &receivedTcpMessage, sizeof(receivedTcpMessage), 0);

      /* CONVERTING RECEIVED DATA TO LITTLE ENDIAN */
      receivedTcpMessage.client_secret_code = ntohs(receivedTcpMessage.client_secret_code);
      receivedTcpMessage.server_secret_code = ntohs(receivedTcpMessage.server_secret_code);
      receivedTcpMessage.length_of_name = ntohs(receivedTcpMessage.length_of_name);

      if (bytes_recd > 0){

        /* ASSIGNING VARIABLES BEFORE SENDING THE DATA TO THE CLIENT */
        tcpMessage.client_secret_code = receivedTcpMessage.client_secret_code;
        tcpMessage.server_secret_code = 14716;

         /* CONVERTING TO BIG ENDIAN */
         tcpMessage.client_secret_code = htons(tcpMessage.client_secret_code);
         tcpMessage.server_secret_code = htons(tcpMessage.server_secret_code);
         tcpMessage.length_of_name = htons(tcpMessage.length_of_name);

         /* SEND MESSAGE */
         bytes_sent = send(sock_connection, &tcpMessage, sizeof(tcpMessage), 0);

         /*
         tcpMessage.client_secret_code = ntohs(tcpMessage.client_secret_code);
         tcpMessage.server_secret_code = ntohs(tcpMessage.server_secret_code);
         tcpMessage.length_of_name = ntohs(tcpMessage.length_of_name);
         */
      }

      /* close the socket */

      close(sock_connection);
   }
}

