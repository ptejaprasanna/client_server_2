all:	udpserver tcpserver client

udpserver: udpserver.c;
	gcc udpserver.c -o udpserver

tcpserver: tcpserver.c;
	gcc tcpserver.c -o tcpserver

client: client.c;
	gcc client.c -o client

clean:
	rm client tcpserver udpserver
