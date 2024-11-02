/* IOCP-TLS CLIENT - create_socket.h
* Author: Tadej S
* Description: This file contains function
* that creates socket and connects it to server.
* Time of writing file: 2nd November 2024*/


#ifndef CREATE_SOCKET_H
#define CREATE_SOCKET_H
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
#include "var.h"

//Function that creates socket and connects it to server.
bool CreateConnectedSocket(SOCKET* pSocket, char* hostname, int port)
{
	//Open TCP socket
	sockaddr_in server_address;
	hostent* Server;

	*pSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*pSocket == INVALID_SOCKET)
	{
		tprint("Error occured while opening socket: %ld\n", WSAGetLastError());
		return false;
	}
	std::cout << "Socket opened successfully.\n";

	Server = gethostbyname(hostname);
	if (Server == NULL)
	{
		closesocket(*pSocket);
		tprint("Error occured: host does not exist.\n");
		return false;
	}

	//Set information about server
	ZeroMemory((char*)&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;

	//Copy data received form gethostbyname() to sockaddr_in
	CopyMemory((char*)&server_address.sin_addr.s_addr,
		(char*)Server->h_addr,
		Server->h_length);
	server_address.sin_port = htons(port);
	
	//perform actual connection to server
	if (connect(*pSocket, reinterpret_cast<const sockaddr*>(&server_address), sizeof(server_address)) == SOCKET_ERROR)
	{
		closesocket(*pSocket);
		tprint("Error occured while connecting to server.\n");
		return false;
	}
	tprint("connected successfully.\n");
	return true;
}

#endif