#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")//winsock library

#define SERVER_PORT 5555
#define LISTEN_TO 5
#define MAX_PEERS 5
#define DEFAULT_BUFLEN 512

int main() {

	WSADATA wsaData;
	int iResult;
	SOCKET connectSocket = INVALID_SOCKET, listenSocket = INVALID_SOCKET,peers[5],newClientSocket,
		connectPeer = INVALID_SOCKET,s;
	struct sockaddr_in server,local,client,peerServer;
    char *hello = "Hello";
	int listenPort, peerListenPort;
	fd_set mySet;//set of socket descriptors
	fd_set read;
	int i,addrlen;
	char str[INET_ADDRSTRLEN];
	char recvbuf[DEFAULT_BUFLEN] = "";
	char messagePeer[50];
	int peerInfo[MAX_PEERS];
	int activity;
	int flag = 0;
	
	
	char sendbuf[50];
	memset(sendbuf, '0', sizeof(sendbuf));

	for (i = 0; i < MAX_PEERS; i++) {

		peers[i] = 0;
	}

	for (i = 0; i < MAX_PEERS; i++) {

		peerInfo[i] = 0;
	}

	//initialize winsock, make sure winsock is also supported on the system
	//WSAStartup to initialize use of WS2_32.dll
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	
	connectSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (connectSocket == INVALID_SOCKET) {

		wprintf(L"connect socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;

	}

	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_PORT);
	//use inet_pton, inet_addr deprecated
	if (inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) <= 0)
	{
		printf("\n inet_pton error occured\n");
		return 1;
	}

	//connect to master server
	iResult = connect(connectSocket, (SOCKADDR*)&server, sizeof(server));
	if (iResult == SOCKET_ERROR) {
		wprintf(L"connect failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	
	
	
	/*// Send an initial buffer to master server
	iResult = send(connectSocket, hello, (int)strlen(hello), 0);
	if (iResult == SOCKET_ERROR) {
		wprintf(L"send failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}*/


	/*// shutdown the connection since no more data will be sent
	iResult = shutdown(connectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
	wprintf(L"shutdown failed with error: %d\n", WSAGetLastError());
	closesocket(connectSocket);
	WSACleanup();
	return 1;
	}*/

	printf("Enter listenig port\n");
	scanf_s("%d", &listenPort);

	listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET) {
		wprintf(L"listen socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;

	}

	//socket structure where the client will listen to
	local.sin_family = AF_INET;
	local.sin_port = htons(listenPort);//this client will listen on this port for incoming connections
	local.sin_addr.s_addr = INADDR_ANY;

	//Bind
	if (bind(listenSocket, (struct sockaddr *)&local, sizeof(local)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	else {

		//send listening port o server
		snprintf(sendbuf, 50, "%d", listenPort);

		iResult = send(connectSocket, sendbuf, (int)strlen(sendbuf), 0);
		if (iResult == SOCKET_ERROR) {
			wprintf(L"send failed with error: %d\n", WSAGetLastError());
			closesocket(connectSocket);
			WSACleanup();
			return 1;
		}
		else {

			printf("I have sent my listening port to server\n");
			printf("Waiting for other peers......\n");
		}
	}
	
	
	while (flag < 2) {

		//clear the socket fd set
		FD_ZERO(&read);

		//add listening socket to fd set
		FD_SET(connectSocket, &read);

		if ((select(0, &read, NULL, NULL, NULL)) == SOCKET_ERROR)
		{
			printf("select call failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		//something happened on the connectSocket
		if (FD_ISSET(connectSocket, &read)) {

			if ((recv(connectSocket, peerInfo, MAX_PEERS*sizeof(int), 0)) > 0) {

				for (i = 0; i < MAX_PEERS; i++) {

					if (peerInfo[i] != 0) {

						flag++;


					}


				}//end for
			}
		}
	}//end while
	
	 

	for (i = 0; i < MAX_PEERS; i++) {

		if (peerInfo[i] != 0) {

			if (peerInfo[i] == listenPort) {

					printf("This is me at port %d\n ", peerInfo[i]);
			}
			else {

					printf("Peer %d is listening at port %d\n", i + 1, peerInfo[i]);
			}


		}


	}//end for

	//listen for incoming connections from peers
	if (listen(listenSocket, LISTEN_TO) < 0) {

		printf("listen failed with error code : %d", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;

	}
	
	addrlen = sizeof(struct sockaddr_in);
	
	printf("Send message to client\n");
	printf("Enter port:\n");
	scanf_s("%d", &peerListenPort);
	printf("Enter message:\n");
	scanf_s("%s", messagePeer,50); // not null terminated

	connectPeer = socket(AF_INET, SOCK_STREAM, 0);
	if (connectPeer == INVALID_SOCKET) {

		wprintf(L"connect socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;

	}

	peerServer.sin_family = AF_INET;
	peerServer.sin_port = htons(peerListenPort);
	//use inet_pton, inet_addr deprecated
	if (inet_pton(AF_INET, "127.0.0.1", &peerServer.sin_addr) <= 0)
	{
		printf("\n inet_pton error occured\n");
		return 1;
	}

	//connect to peer
	iResult = connect(connectPeer, (SOCKADDR*)&peerServer, sizeof(peerServer));
	if (iResult == SOCKET_ERROR) {
		wprintf(L"connect failed with error: %d\n", WSAGetLastError());
		closesocket(connectPeer);
		WSACleanup();
		return 1;
	}

	//send message to peer
	iResult = send(connectPeer, messagePeer, (int)strlen(messagePeer), 0);
	if (iResult == SOCKET_ERROR) {
		wprintf(L"send failed with error: %d\n", WSAGetLastError());
		closesocket(connectPeer);
		WSACleanup();
		return 1;
	}


  
	while (TRUE) {

		//clear the socket fd set
		FD_ZERO(&mySet);

		//add listening socket to fd set
		FD_SET(listenSocket, &mySet);
		FD_SET(connectSocket, &mySet);
		FD_SET(connectPeer, &mySet);

		//loop through all sockets and add to myset
		for (i = 0; i < MAX_PEERS; i++)
		{
			if (peers[i] > 0)
			{
				FD_SET(peers[i], &mySet);
			}
		}//end for

		 //wait for an activity on any of the sockets, timeout is NULL , so wait indefinitely
		activity = select(0, &mySet, NULL, NULL, NULL);

		if (activity == SOCKET_ERROR)
		{
			printf("select call failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		//something happened on the connectSocket
		if (FD_ISSET(connectSocket, &mySet)) {

			if (recv(connectSocket, peerInfo, MAX_PEERS*sizeof(int), 0) > 0) {
				printf("\nThis is an update from the server\n");
				for (i = 0; i < MAX_PEERS; i++) {					

					if (peerInfo[i] != 0)					
					printf("Listening peer port at index %d is %d\n", i, peerInfo[i]);

				}
			}


		}//end if FD_ISSET
		
		if (FD_ISSET(connectPeer, &mySet)) {

		if ((iResult = recv(connectPeer, recvbuf, DEFAULT_BUFLEN, 0)) > 0) {

		
			recvbuf[iResult] = '\0';
		    printf("Acknowledgement %s : \n",recvbuf);

		
		}


		}//end if FD_ISSET

		//If something happened on the listening socket , then its an incoming connection
		if (FD_ISSET(listenSocket, &mySet)) {

			if ((newClientSocket = accept(listenSocket, (struct sockaddr *)&client, (int *)&addrlen)) < 0)
			{
				perror("accept");
				exit(EXIT_FAILURE);
			}

			//inform user of socket number - used in send and receive commands
			printf("\nNew connection , socket id is %d ,ip %s , port : %d \n",
				newClientSocket, inet_ntop(AF_INET, &(client.sin_addr), str, INET_ADDRSTRLEN),
				ntohs(client.sin_port));

			//add new socket to array of sockets
			for (i = 0; i < MAX_PEERS; i++)
			{
				if (peers[i] == 0)
				{
					peers[i] = newClientSocket;
					printf("Adding to list of sockets at index %d \n", i);
					break;
				}
			}//end for

		}//end if FD_ISSET

		
		for (i = 0; i < MAX_PEERS; i++)
		{
			s = peers[i];

			//if client present in mySet             
			if (FD_ISSET(s, &mySet))
			{
				//get details of the client
				getpeername(s, (struct sockaddr *)&client, (int *)&addrlen);

				//Check if it was for closing , and also read the incoming message
				//recv does not place a null terminator at the end of the string (whilst printf %s assumes there is one).

				iResult = recv(s, recvbuf, DEFAULT_BUFLEN, 0);

				if (iResult == SOCKET_ERROR)
				{
					int error_code = WSAGetLastError();
					if (error_code == WSAECONNRESET)
					{
						//Somebody disconnected , get his details and print
						printf("Host disconnected unexpectedly , ip %s,  port %d \n",
							inet_ntop(AF_INET, &(client.sin_addr),
								str, INET_ADDRSTRLEN), ntohs(client.sin_port));

						//Close the socket and mark as 0 in list for reuse
						closesocket(s);
						peers[i] = 0;

					}
					else
					{
						printf("recv failed with error code : %d", error_code);
					}
				}
				if (iResult == 0)
				{
					//Somebody disconnected , get his details and print
					printf("Host disconnected , ip %s,  port %d \n",
						inet_ntop(AF_INET, &(client.sin_addr), str, INET_ADDRSTRLEN), ntohs(client.sin_port));

					//Close the socket and mark as 0 in list for reuse
					closesocket(s);
					peers[i] = 0;
				}

				else
				{
					//add null character, if you want to use with printf/puts or other string handling functions
					recvbuf[iResult] = '\0';
					printf("Bytes received is %d\n", iResult);
					printf("Message from client ip %s, port %d - %s \n",
						inet_ntop(AF_INET, &(client.sin_addr), str, INET_ADDRSTRLEN)
						, ntohs(client.sin_port), recvbuf);

					char *ack = "ACK";

					send(s, ack, (int)strlen(ack), 0);
				}
			}//if FD_ISSET

		}//end for



	}//end while foer accepting

	


	return 0;
}//end main
