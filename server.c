#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WS2tcpip.h>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define MAXPENDING  5
#define MAXRECV 1024
#define MAX_CLIENTS 5
#define MASTER_PORT 5555


int main(int argc, char *argv[]) {

	WSADATA wsa;
	SOCKET sockid, newsockid, client_socket[MAX_CLIENTS], s;
	struct sockaddr_in serv_addr, cli_addr;
	char *message = "Wallapa client, i am server";
	int addrLen;
	char str[INET_ADDRSTRLEN];
	int activity, i;
	int valread;
	int clientListenPorts[MAX_CLIENTS];
	int in = 0;
	

	//set of socket descriptors
	fd_set readfds;
	//1 extra for null character, string termination
	char *buffer;
	buffer = (char*)malloc((MAXRECV + 1) * sizeof(char));


	for (i = 0; i < MAX_CLIENTS; i++)
	{
		client_socket[i] = 0;
	}

	for (i = 0; i < MAX_CLIENTS; i++) {

		clientListenPorts[i] = 0;
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}
	else {

		printf("Winsock initialized.\n");
	}



	//create socket for incoming connections
	sockid = socket(AF_INET, SOCK_STREAM, 0);

	if (sockid < 0) {

		printf("socket() failed\n", WSAGetLastError());

	}



	//prepare the socket structure	 	
	serv_addr.sin_family = AF_INET; //internet address family
	serv_addr.sin_addr.s_addr = INADDR_ANY;//any incoming interface
	serv_addr.sin_port = htons(MASTER_PORT);//

	if (bind(sockid, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {

		printf("bind() failed : %d\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}



	//now start listening for the clients
	if (listen(sockid, MAXPENDING) < 0) {

		printf("listen() failed : %d\n", WSAGetLastError());
		exit(EXIT_FAILURE);

	}


	printf("Chilling for clients to connect\n");

	addrLen = sizeof(struct sockaddr_in);


	while (TRUE) {

		//clear the socket set
		FD_ZERO(&readfds);

		//add master socket set
		FD_SET(sockid, &readfds);

		//add child sockets

		for (i = 0; i < MAX_CLIENTS; i++)
		{
			s = client_socket[i];
			if (s > 0)
			{
				FD_SET(s, &readfds);
			}
		}

		//wait for an activity on any of the sockets, timeout is NULL , so wait indefinitely
		activity = select(0, &readfds, NULL, NULL, NULL);

		if (activity == SOCKET_ERROR)
		{
			printf("select call failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		//If something happened on the master socket , then its an incoming connection
		if (FD_ISSET(sockid, &readfds))
		{
			if ((newsockid = accept(sockid, (struct sockaddr *)&cli_addr, (int *)&addrLen)) < 0)
			{
				perror("accept");
				exit(EXIT_FAILURE);
			}

			//inform user of socket number - used in send and receive commands
			printf("\nNew connection , socket id is %d ,ip %s , port : %d \n",
				newsockid, inet_ntop(AF_INET, &(cli_addr.sin_addr), str, INET_ADDRSTRLEN), ntohs(cli_addr.sin_port));


			/*//send new connection greeting message
			if (send(newsockid, message, (int)strlen(message), 0) != strlen(message))
			{
				perror("send failed");

			}*/



			//add new socket to array of sockets
			for (i = 0; i < MAX_CLIENTS; i++)
			{
				if (client_socket[i] == 0)
				{
					client_socket[i] = newsockid;
					printf("Adding to list of sockets at index %d \n", i);
					break;
				}
			}//end for
	

		}//end if

		/*for (i = 0; i < MAX_CLIENTS; i++) {

			if (client_socket[i] != 0)
				send(client_socket[i], (char*)client_socket, MAX_CLIENTS*sizeof(int), 0);
		}*/
	
		for (i = 0; i < MAX_CLIENTS; i++)
		{
			s = client_socket[i];
			//if client presend in read sockets             
			if (FD_ISSET(s, &readfds))
			{
				//get details of the client
				getpeername(s, (struct sockaddr *)&cli_addr, (int *)&addrLen);

				//Check if it was for closing , and also read the incoming message
				//recv does not place a null terminator at the end of the string (whilst printf %s assumes there is one).

			
				valread = recv(s, buffer, MAXRECV, 0);
				
				if (valread == SOCKET_ERROR)
				{
					int error_code = WSAGetLastError();
					if (error_code == WSAECONNRESET)
					{
						//Somebody disconnected , get his details and print
						printf("Host disconnected unexpectedly , ip %s,  port %d \n",
							inet_ntop(AF_INET, &(cli_addr.sin_addr), str, INET_ADDRSTRLEN), ntohs(cli_addr.sin_port));

						//Close the socket and mark as 0 in list for reuse
						closesocket(s);
						client_socket[i] = 0;
						clientListenPorts[i] = 0;

					}
					else
					{
						printf("recv failed with error code : %d", error_code);
					}
				}
				if (valread == 0)
				{
					//Somebody disconnected , get his details and print
					printf("Host disconnected , ip %s,  port %d \n",
						inet_ntop(AF_INET, &(cli_addr.sin_addr), str, INET_ADDRSTRLEN), ntohs(cli_addr.sin_port));

					//Close the socket and mark as 0 in list for reuse
					closesocket(s);
					client_socket[i] = 0;
					clientListenPorts[i] = 0;
				}

				else
				{
					//add null character, if you want to use with printf/puts or other string handling functions
					//buffer[valread] = '\0';

					

					//send array to client
					//send(client_socket[i], (char*)clientListenPorts, MAX_CLIENTS*sizeof(char), 0);
					//send(client_socket[i], message, (int)strlen(message), 0) != strlen(message);
					//send_strings(client_socket[i], clientListenPorts, 5, 0);

					
					sscanf_s(buffer, "%d", &in);
					clientListenPorts[i] = in;

					for (i = 0; i < MAX_CLIENTS; i++) {

						if (client_socket[i] != 0)
							send(client_socket[i] , (char*)clientListenPorts, MAX_CLIENTS*sizeof(int), 0);
					}
					

					printf("Bytes received is %d\n", valread);
					printf("Message from client ip %s, port %d - %d \n", 
						inet_ntop(AF_INET, &(cli_addr.sin_addr), str, INET_ADDRSTRLEN)
						, ntohs(cli_addr.sin_port), in);

				}
			}//if FD_ISSET
		}//end for


	}//end while

	closesocket(s);
	WSACleanup();
	return 0;
}

