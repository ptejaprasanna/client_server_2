
#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset, memcpy, and strlen */
#include <netdb.h>          /* for struct hostent and gethostbyname */
#include <sys/socket.h>     /* for socket, connect, send, and recv */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */

/* FUNCTION TO CHECK FOR ALREADY EXISTING PORT NUMBER IN THE FILE */
int portCheck(unsigned short port);

/* STRUCTURE FOR COMMUNICATING WITH THE TCP SERVER */
struct message_tcp{
  unsigned short client_secret_code;
  unsigned short server_secret_code;
  unsigned short length_of_name;
};

/* STRUCTURES FOR COMMUNICATING WITH THE UDP SERVER */
struct udp_message_send{
  unsigned short client_secret_code;
  unsigned short server_secret_code;
  unsigned short length_of_name;
  char name[15];
};
struct udp_message_receive{
  unsigned short client_secret_code;
  unsigned short server_secret_code;
  unsigned short length_of_name;
  char name[80];
};

#define STRING_SIZE 1024

int main(void) {

   int sock_client_tcp;  /* Socket used by client */
   int sock_client_udp;
   int client_port;

   struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
   struct sockaddr_in client_addr;
   struct hostent * server_hp;      /* Structure to store server's IP
                                        address */
   /* DECLARING TCP STRUCTURE VARIABLES */
   struct message_tcp tcpMessage, receivedTcpMessage;

   /* INITIALIZING TCP STRUCTURE VARIABLES TO ZERO */
   tcpMessage.client_secret_code = 0;
   receivedTcpMessage.client_secret_code = 0;
   tcpMessage.server_secret_code = 0;
   receivedTcpMessage.server_secret_code = 0;
   tcpMessage.length_of_name = 0;
   receivedTcpMessage.length_of_name = 0;

   /* DECLARING UDP STRUCTURE VARIABLES */
   struct udp_message_send udpMessage;
   struct udp_message_receive receivedUdpMessage;

   char server_hostname[STRING_SIZE]; /* Server's hostname */
   unsigned short server_port;  /* Port number used by server (remote port) */
   int bytes_sent, bytes_recd; /* number of bytes sent or received */
   /* CREATING A FILE  */
   FILE * cFile;
   cFile = fopen("ClientInfo", "a");
   fclose(cFile);
   /* VARIABLE FOR CHECKING ALREADY EXISTING PORT NUMBERS IN THE FILE */
   unsigned int flag;
   /* PORT DISCOVERY */
   for(server_port = 48000; server_port <= 48999; server_port++){
       
     /* INITIALIZING UDP STRUCTURE VARIABLES TO ZERO */
     udpMessage.client_secret_code = 0;
     receivedUdpMessage.client_secret_code = 0;
     udpMessage.server_secret_code = 0;
     receivedUdpMessage.server_secret_code = 0;
     udpMessage.length_of_name = 0;
     receivedUdpMessage.length_of_name = 0;
     memset(receivedUdpMessage.name, 0, 80);
     memset(udpMessage.name, 0, 15);

     /* IF PORT ALREADY DISCOVERED */
     int checkPort = portCheck(server_port);
     if(checkPort == 1){
      continue;
     }

     else{
       if ((sock_client_tcp = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
          perror("Client: can't open stream socket");
          exit(1);
       }

       /* Note: there is no need to initialize local client address information
                unless you want to specify a specific local port
                (in which case, do it the same way as in udpclient.c).
                The local address initialization and binding is done automatically
                when the connect function is called later, if the socket has not
                already been bound. */

       /* initialize server address information */

       //printf("Enter hostname of server: ");
       //scanf("%s", server_hostname);
       memcpy(server_hostname, "cisc650", 7);
       if ((server_hp = gethostbyname(server_hostname)) == NULL) {
          perror("Client: invalid server hostname");
          close(sock_client_tcp);
          exit(1);
       }

       /* Clear server address structure and initialize with server address */
       memset(&server_addr, 0, sizeof(server_addr));
       server_addr.sin_family = AF_INET;
       memcpy((char *)&server_addr.sin_addr, server_hp->h_addr,
                                        server_hp->h_length);
       server_addr.sin_port = htons(server_port);

        /* connect to the server */
       if(connect(sock_client_tcp, (struct sockaddr *) &server_addr,
                                        sizeof (server_addr)) == -1) {
          continue;                          //SKIP IF SERVER DOES NOT EXIST
          perror("Client: can't connect to server");
          close(sock_client_tcp);
          exit(1);
       }
       printf("\nPort : %d\n", server_port);
       /* CONVERTING TO BIG ENDIAN */
       tcpMessage.client_secret_code = 35624;
       tcpMessage.client_secret_code = htons(tcpMessage.client_secret_code);
       tcpMessage.server_secret_code = htons(tcpMessage.server_secret_code);
       tcpMessage.length_of_name = htons(tcpMessage.length_of_name);

      /*SEND MESSAGE */
       bytes_sent = send(sock_client_tcp, &tcpMessage, sizeof(tcpMessage), 0);

       /* CONVERTING BACK TO LITTLE ENDIAN FOR OTHER MANIPULATIONS */
       tcpMessage.client_secret_code = ntohs(tcpMessage.client_secret_code);
       tcpMessage.server_secret_code = ntohs(tcpMessage.server_secret_code);
       tcpMessage.length_of_name = ntohs(tcpMessage.length_of_name);

       /* get response from server */

       bytes_recd = recv(sock_client_tcp, &receivedTcpMessage, sizeof(tcpMessage), 0);

       /* CONVERT RECEIVED MESSAGE TO LITTLE ENDIAN */
       receivedTcpMessage.client_secret_code = ntohs(receivedTcpMessage.client_secret_code);
       receivedTcpMessage.server_secret_code = ntohs(receivedTcpMessage.server_secret_code);
       receivedTcpMessage.length_of_name = ntohs(receivedTcpMessage.length_of_name);

       /* close the socket */

       close (sock_client_tcp);

       /* UDP MESSAGE */
       /* open a socket */

       if ((sock_client_udp = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
          perror("Client: can't open datagram socket\n");
          exit(1);
       }

       /* Note: there is no need to initialize local client address information
                unless you want to specify a specific local port.
                The local address initialization and binding is done automatically
                when the sendto function is called later, if the socket has not
                already been bound.
                The code below illustrates how to initialize and bind to a
                specific local port, if that is desired. */

       /* initialize client address information */

       client_port = 0;   /* This allows choice of any available local port */

       /* Uncomment the lines below if you want to specify a particular
                 local port: */
       /*
       printf("Enter port number for client: ");
       scanf("%hu", &client_port);
       */

       /* clear client address structure and initialize with client address */
       memset(&client_addr, 0, sizeof(client_addr));
       client_addr.sin_family = AF_INET;
       client_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* This allows choice of
                                            any host interface, if more than one
                                            are present */
       client_addr.sin_port = htons(client_port);

       /* bind the socket to the local client port */

       if (bind(sock_client_udp, (struct sockaddr *) &client_addr,
                                        sizeof (client_addr)) < 0) {
          perror("Client: can't bind to local address\n");
          close(sock_client_udp);
          exit(1);
       }

       /* end of local address initialization and binding */

       /* initialize server address information */

       if ((server_hp = gethostbyname(server_hostname)) == NULL) {
          perror("Client: invalid server hostname\n");
          close(sock_client_udp);
          exit(1);
       }

       /* Clear server address structure and initialize with server address */
       memset(&server_addr, 0, sizeof(server_addr));
       server_addr.sin_family = AF_INET;
       memcpy((char *)&server_addr.sin_addr, server_hp->h_addr,
                                        server_hp->h_length);
       server_addr.sin_port = htons(server_port);

       /* ASSIGNING VALUES TO UDP STRUCTURE VARIABLE */

       udpMessage.client_secret_code = tcpMessage.client_secret_code;
       udpMessage.server_secret_code = receivedTcpMessage.server_secret_code;
       memcpy(udpMessage.name, "teja-parlapalli", sizeof(receivedUdpMessage.name));
       udpMessage.length_of_name = strlen(udpMessage.name);

       /* CONVERTING DATA TO BIG ENDIAN */
       udpMessage.client_secret_code = htons(udpMessage.client_secret_code);
       udpMessage.server_secret_code = htons(udpMessage.server_secret_code);
       udpMessage.length_of_name = htons(udpMessage.length_of_name);

       /* SEND MESSAGE */
       bytes_sent = sendto(sock_client_udp, &udpMessage, sizeof(udpMessage) - 1, 0,
                (struct sockaddr *) &server_addr, sizeof (server_addr));

       /* CONVERTING DATA TO LITTLE ENDIAN */
       udpMessage.client_secret_code = ntohs(udpMessage.client_secret_code);
       udpMessage.server_secret_code = ntohs(udpMessage.server_secret_code);
       udpMessage.length_of_name = ntohs(udpMessage.length_of_name);

       /* get response from server */
       bytes_recd = recvfrom(sock_client_udp, &receivedUdpMessage, sizeof(receivedUdpMessage), 0,
                    (struct sockaddr *) 0, (int *) 0);

       /* CONVERTING RECEIVED DATA TO LITTLE ENDIAN */
       receivedUdpMessage.client_secret_code = ntohs(receivedUdpMessage.client_secret_code);
       receivedUdpMessage.server_secret_code = ntohs(receivedUdpMessage.server_secret_code);
       receivedUdpMessage.length_of_name = ntohs(receivedUdpMessage.length_of_name);

       char  receivedname[80];
       /* CHECKS IF NAME IS RECEIVED */
       if(receivedUdpMessage.length_of_name > 0){
	memset(receivedname, 0, sizeof(receivedname));
	memcpy(receivedname, receivedUdpMessage.name, receivedUdpMessage.length_of_name);
   	
        FILE * wFile;
       
       /* OPENING FILE APPEND MODE */
        wFile = fopen("ClientInfo", "a");
        fprintf(wFile, "%d,%d,%s\n", server_port, receivedUdpMessage.server_secret_code, receivedname);
        fclose(wFile);
       }
      
       printf("\nPort Number %d", server_port);
       printf("\nTCP Message sent:     %d", tcpMessage.client_secret_code);
       printf("\nTCP Message received: %d, %d", receivedTcpMessage.client_secret_code, receivedTcpMessage.server_secret_code);
       printf("\nUDP Message sent:     %d, %d, %s", udpMessage.client_secret_code, udpMessage.server_secret_code, udpMessage.name);
       printf("\nUDP Message received: %d, %d, %s", receivedUdpMessage.client_secret_code, receivedUdpMessage.server_secret_code, receivedname);
       printf("\n");
       close (sock_client_udp);
     }
  }
  //fclose(wFile);
}

/* FUNCTION TO CHECK FOR ALREADY PORT NUMBERS IN THE FILE BEFORE WRITING DATA ONTO THE FILE */
int portCheck(unsigned short port){
  FILE * rFile;

  /* OPENS FILE IN READ MODE */
  rFile = fopen("ClientInfo", "r");

  /*TEMPORARY VARIABLES */
  char buf[100];
  char temp[100];
  char text_port[5];
  unsigned short e_port_number = 0;

  /* CHECKS FOR PORT NUMBER AT EVERY LINE IN THE FILE */
  while (fgets(buf, sizeof(buf), rFile)) {
       sprintf(temp, "%s", buf);
   /* EXTRACTING PORT NUMBER */
       int i;
       for (i = 0; i < strlen(temp); i++){
         if(temp[i] == ','){
           break;
         }
   /* sprintf COPIES OUTPUT TO THE CHARACTER ARRAY WITHOUT DISPLAYING IT ON THE CONSOLE */
         sprintf(text_port + i, "%c", temp[i]);
           if(temp[i+1] == ','){

             /* CONVERTS STRING TO INTEGER */
             e_port_number = atoi (text_port);

             if ( e_port_number == port){
                return 1;
		break;
		}  
	 }
        }
       /* END OF EXTRACTION */
  }
  fclose(rFile);

  /* IF PORT ALREADY EXISTS IN THE FILE */
  if(e_port_number == port){
    return 1;
  }
  else{
    return 0;
  }
}

