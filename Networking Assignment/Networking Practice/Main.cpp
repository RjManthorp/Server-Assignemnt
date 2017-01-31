#include <iostream>
#include <WinSock2.h>
#include <thread>
#include <vector>
#include <string>
#pragma comment(lib, "Ws2_32.lib")

const unsigned short PORT = 1111;
const int MAX_CLIENTS = 5;
const int DEFAULT_BUFLEN = 512;

struct client_type // Track individual clients
{
	int id;
	SOCKET socket;
};

int processClient(client_type &new_client, std::vector<client_type> &client_array, std::thread &thread)
{

	std::string msg = "";
	char tempmsg[DEFAULT_BUFLEN];

	while (1)
	{
		memset(tempmsg, 0, DEFAULT_BUFLEN);
		int result = recv(new_client.socket, tempmsg, DEFAULT_BUFLEN, 0);

		if (result != SOCKET_ERROR)
		{
			if (strcmp("", tempmsg))
				msg = " Client #" + std::to_string(new_client.id) + ": " + tempmsg;

			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (client_array[i].socket != INVALID_SOCKET)
				{
					if (new_client.id != i)
					{
						result = send(client_array[i].socket,
							msg.c_str(),
							strlen(msg.c_str()), 0);
					}
				}
			}
		}
		else
		{
			msg = " Client #" + std::to_string(new_client.id) + " disconnected";
			std::cout << msg << std::endl;
			closesocket(new_client.socket);
			closesocket(client_array[new_client.id].socket);
			client_array[new_client.id].socket = INVALID_SOCKET;

			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (client_array[i].socket != INVALID_SOCKET)
				{
					if (new_client.id != i)
					{
						result = send(client_array[i].socket,
							msg.c_str(),
							strlen(msg.c_str()), 0);
					}
				}
			}
			break;
			thread.detach();
			return 0;
		}

	}
}



int main()
{
	printf("******************************************\n");
	printf("**               Chat Room              **\n");
	printf("******************************************\n");
	system("color 4f");

	WORD wVersionRequested = MAKEWORD(2, 2); // main function for initializing winsock
	WSADATA wsaData;

	printf("Initializing WinSock..."); // wsas startup returns anything but 0 the end it
	int error = WSAStartup(wVersionRequested, &wsaData);

	if (error != 0)
	{
		return 0;

	}

	SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); //requesting a socket

	if (sock == INVALID_SOCKET)
	{
		printf("Could not create socket : %d\n", WSAGetLastError());
	}

	struct sockaddr_in sa, client;
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = ADDR_ANY;
	sa.sin_port = htons(PORT);

	error = bind(sock, (struct sockaddr*)&sa, sizeof(sa));
	if (error == SOCKET_ERROR)
	{
		printf("Bind failed with error code: %d", WSAGetLastError());
		return 0;
	}

	if (listen(sock, 256) == SOCKET_ERROR) // socket to listen mode
	{
		printf("Failed to set socket to listen with error code: %d", WSAGetLastError());
		return 0;
	}
	


	std::vector<client_type> clients(MAX_CLIENTS);// vector to store clients and an array of threads to process them
	std::thread threads[MAX_CLIENTS];
	int num_clients = 0;
	int temp_id = -1;


	for (int i = 0; i < MAX_CLIENTS; i++) //keep checking the listening socket for incomming conections and pass them to process threads if there is space
	{
		clients[i] = { -1, INVALID_SOCKET };
	}


	int size = sizeof(client);
	std::string msg;
	while (1)
	{
		SOCKET clientSock = accept(sock, (sockaddr *)&client, &size);
		if (clientSock == INVALID_SOCKET) continue;

		num_clients = -1;
		temp_id = -1;
		for (int i = 0; i < MAX_CLIENTS; i++) //search client list for empty slot
		{
			if (clients[i].socket == INVALID_SOCKET && temp_id == -1)
			{
				clients[i].socket = clientSock;
				clients[i].id = i;
				temp_id = i;
			}
			if (clients[i].socket != INVALID_SOCKET)
				num_clients++;
		}

		if (temp_id != -1)
		{
			std::cout << "Client #" << clients[temp_id].id << " accepted." << std::endl;
			msg = std::to_string(clients[temp_id].id);
			send(clients[temp_id].socket, msg.c_str(), strlen(msg.c_str()), 0);
			threads[temp_id] = std::thread(processClient, std::ref(clients[temp_id]), std::ref(clients), std::ref(threads[temp_id]));
		}
		else
		{
			msg = "Server is full";
			send(clientSock, msg.c_str(), strlen(msg.c_str()), 0);
			std::cout << msg << std::endl;
		}
	}

	closesocket(sock);
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		threads[i].detach();
		closesocket(clients[i].socket);
	}
	WSACleanup();
	return 0;
}