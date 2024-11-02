/* IOCP-TLS SERVER -  Initialize.h
* Author: Tadej S
* Description: This file contains
* accept thread, function that processes
* incoming client connection and function that 
* associates newly connected client with IOCP.
* Time of writing file: 1st November 2024*/

#ifndef THREADS_H
#define THREADS_H
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <vector>
#include "var.h"
#include "Client.h"

//Function that associates connected client with I/O Completion Port.
bool AssociateWithIOCP(Client* pClient)
{
	//Associate the socket with IOCP
	HANDLE hTemp = CreateIoCompletionPort((HANDLE)pClient->GetSocket(), var::hIOCompletionPort, (DWORD)pClient, 0);
	if (hTemp == NULL)
	{
		tprint("Error occured while executing CreateIoCompletionPort().\n");
		RemoveFromClientListAndFreeMemory(pClient);
		return false;
	}
	return true;
}

//Function that processes incoming connection
void AcceptConnection(SOCKET ListenSocket)
{
	//Accept client
	sockaddr_in ClientAddress;
	int nClientLength = sizeof(ClientAddress);
	SOCKET Socket = accept(ListenSocket, (sockaddr*)&ClientAddress, &nClientLength);

	if (Socket == INVALID_SOCKET)
	{
		tprint("Accept failed: %ld", WSAGetLastError());
	}
	tprint("Client connected from: %s\n", inet_ntoa(ClientAddress.sin_addr));

	//Create a new Client object for this newly accepted client
	Client* pClient = new Client;
	pClient->SetOpCode(OP_READ);
	pClient->SetSocket(Socket);
	AddToClientList(pClient);

	if (!AssociateWithIOCP(pClient))
	{
		tprint("Failed to associate client with I/O Completion Port.\n");
	}

	pClient->SetOpCode(OP_WRITE);
	WSABUF* p_wbuf = pClient->GetWSABUFPtr();
	OVERLAPPED* p_ol = pClient->GetOLPtr();
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;

	/*Post initial recv. Posting initial recv
	* in worker thread can cause scalability
	* issues.*/
	int nBytesRecv = WSARecv(pClient->GetSocket(), p_wbuf, 1,
		&dwBytes, &dwFlags, p_ol, NULL);

	if ((nBytesRecv == SOCKET_ERROR) && (WSAGetLastError() != WSA_IO_PENDING))
	{
		tprint("Error in initial recv.\n");
	}
}

//Thread that will look for clients
DWORD WINAPI AcceptThread(LPVOID data)
{
	SOCKET ListenSocket = (SOCKET)data;
	WSANETWORKEVENTS wsaevents;
	
	//Main accept loop
	while (WaitForSingleObject(var::hShutdownEvent, 0) != WAIT_OBJECT_0)
	{
		if (WSAWaitForMultipleEvents(1, &var::hAcceptEvent, FALSE, WAIT_TIMEOUT_INTERVAL, FALSE) != WSA_WAIT_TIMEOUT)
		{
			WSAEnumNetworkEvents(ListenSocket, var::hAcceptEvent, &wsaevents);
			if ((wsaevents.lNetworkEvents & FD_ACCEPT) && (wsaevents.iErrorCode[FD_ACCEPT_BIT] == 0))
			{
				//Process it
				AcceptConnection(ListenSocket);
			}
		}
	}

	return 0;
}

#endif