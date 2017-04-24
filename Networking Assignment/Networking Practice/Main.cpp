#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>
#include <thread>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>
#include <sstream>
#include <iterator>

#pragma comment(lib, "Ws2_32.lib")



const unsigned short PORT = 1111;
const int MAX_CLIENTS = 5;
const int DEFAULT_BUFLEN = 512;

char UsernameChar[DEFAULT_BUFLEN] = "";
int StartSpace = 0;

const int USERNAME_MIN_LENGTH = 10;
const int USERNAME_MAX_LENGTH = 30;


std::string clientName = "Client #";

struct client_type // Track individual clients
{
	int id;
	SOCKET socket;
	std::string userName;
};

void clean(std::string &message)
{
	message.erase(std::remove(message.begin(), message.end(), '\n'), message.end());
	message.erase(std::remove(message.begin(), message.end(), '\r'), message.end());
}

template<typename Out>
void split(const std::string &s, char delim, Out result)
{
	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim))
	{
		*(result++) = item;
	}
}


std::vector<std::string> split(const std::string &s, char delim)
{
	std::vector<std::string> elems;
	split(s, delim, std::back_inserter(elems));
	return elems;
}

int processClient(client_type &new_client, std::vector<client_type> &client_array, std::thread &thread)
{
	time_t now = time(0);
	char* dt = ctime(&now);
	std::string msg = "";
	char tempmsg[DEFAULT_BUFLEN];

	while (1)
	{
		memset(tempmsg, 0, DEFAULT_BUFLEN);
		int result = recv(new_client.socket, tempmsg, DEFAULT_BUFLEN, 0);

		if (result != SOCKET_ERROR)
		{
			std::string message = tempmsg;
			clean(message);
			if (!(tempmsg[0] == 13 && tempmsg[1] == 10 && tempmsg[2] == 0))
			{
				if (message.length() > 0)
				{
					if (message[0] == 47)
					{
						////////////////////////////////////////
						int ban = strncmp("/ban", tempmsg, 4);
						if (strncmp("/ban", tempmsg, 4) == 0)
						{
							std::vector<std::string> splitString = split(tempmsg, ' ');
							std::string BanName = splitString.back(); //passes through part of the string after a "space" 
						
							{
								for (int i = 0; i < MAX_CLIENTS; i++)
								{
									if (client_array[i].userName == BanName)
									{
										std::cout << BanName << " banned" << std::endl;
										send(new_client.socket, BanName.c_str(), strlen(clientName.c_str()), 0);

										closesocket(client_array[i].socket);
									}
								}
							}
							
						}
						/////////////////////////////////////////
						int help = strncmp("/help", tempmsg, 5);
						if (strncmp("/help", tempmsg, 5) == 0)
						{
							std::vector<std::string> splitString = split(tempmsg, ' ');

							{
								std::string helpCommand = "Commands~\n " "/setname (name) \n" "/ban (user) \n";
								msg = helpCommand;
								send(new_client.socket, msg.c_str(), strlen(msg.c_str()), 0);	
							}

						}
						////////////////////////////////////////
						int setname = strncmp("/setname", tempmsg, 8);

						if (strncmp("/setname", tempmsg, 8) == 0)
						{
							int length = std::string(tempmsg).length();
							if (length > USERNAME_MIN_LENGTH & length < USERNAME_MAX_LENGTH)
							{
								std::vector<std::string> splitString = split(tempmsg, ' ');
								std::string Username = splitString.back();
								if (Username !="/setname")
								std::cout << Username << " has joined the room" << std::endl;
								new_client.userName = Username;
								msg = "Hello " + Username;
								send(new_client.socket, msg.c_str(), strlen(msg.c_str()), 0);
							}
							else
							{
								std::cout << " Illegal Username" << std::endl;
								msg = "Illegal Username - Username but be at least 2 characters and no more than 30 characters\r\n";
								send(new_client.socket, msg.c_str(), strlen(msg.c_str()), 0);
							}
						}
					}
					else
					{
						msg = new_client.userName + ": " /*+ "[" + dt + "]"*/ + message + "\r\n";
						send(new_client.socket, message.c_str(), strlen(message.c_str()), 0);
						
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
				}
			}
		}
		else
		{
			msg = (new_client.userName) + " has disconnected";
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
	printf("\n");
	system("color 4f");

	time_t now = time(0);
	char* dt = ctime(&now);
	std::cout << "Time of login of the server - " << dt << std::endl;


	WORD wVersionRequested = MAKEWORD(2, 2); // main function for initializing winsock
	WSADATA wsaData;

	printf("Initializing WinSock...\n"); // wsas startup returns anything but 0 the end it
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

