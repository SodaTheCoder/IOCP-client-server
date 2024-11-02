/* IOCP-TLS SERVER -  Initialize.h
* Author: Tadej S
* Description: This file contains 
* functions to initialize libraries,
* I/O Completion port, create listening socket,
* and start looking for clients.
* Time of writing file: 1st November 2024*/

#ifndef SERVER_IOCP_H
#define SERVER_IOCP_H
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <conio.h>
#include "Accept.h"
#include "var.h"
#include "Worker_thread.h"
#include "cleanup.h"
#define WORKER_THREADS_PER_PROCESSOR 2

bool StartAccepting(SOCKET);

//Function to get number of CPU threads. Will be used to determine how much worker threads to create.
int GetNoOfProcessors()
{
	static int nProcessors = 0;
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	nProcessors = si.dwNumberOfProcessors;
	return nProcessors;
}

//Function to initialize I/O Completion ports. Creates IOCP and Worker Threads.
bool InitializeIOCP()
{
	//Create IO completion port
	var::hIOCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (var::hIOCompletionPort == NULL)
	{
		std::cout << "CreateIoCompletionPort() failed with error: " << WSAGetLastError() << "\n";
		return false;
	}
	std::cout << "Created I/O Completion port successfully.\n";

	DWORD nThreadID;

	//Create worker threads
	for (int i = 0; i < var::nThreads; i++)
	{
		var::phWorkerThreads[i] = CreateThread(0, 0, WorkerThread, (void*)(i + 1), 0, &nThreadID);
	}

	return true;
}

// Function to initialize WinSock, critical sections and shutdown event.
bool Initialize()
{
	//Initialize WinSock
	WSADATA wsaData;
	int iResult = 0;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cout << "WSAStartup() failed with error code: " << iResult << "\n";
		return false;
	}
	std::cout << "WinSock initialized successfully.\n";

	//Determine number of CPU threads and number of worker threads that will be created.
	int cpu_threads = GetNoOfProcessors();
	var::nThreads = cpu_threads * WORKER_THREADS_PER_PROCESSOR;
	std::cout << "Server has " << cpu_threads << " cpu threads.\n";
	std::cout << var::nThreads << " worker threads will be created.\n";
	var::phWorkerThreads = new HANDLE[var::nThreads];

	//Initialize critical sections
	InitializeCriticalSection(&var::csConsole);
	InitializeCriticalSection(&var::csClientList);
	std::cout << "Critical sections initialized successfully.\n";

	//Create shutdown event
	var::hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	std::cout << "Shutdown event created successfully.\n";

	//Initialize IOCP
	if (!InitializeIOCP())
	{
		std::cout << "InitializeIOCP() failed.\n";
	}
	std::cout << "IOCP initialized successfully.\n";

	return true;
}

//Function to create listen socket, bind it and start listening
bool CreateSocket(int port)
{
	SOCKET ListenSocket;
	sockaddr_in ServerAddress;

	//Creating overlapped listen socket
	ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (ListenSocket == INVALID_SOCKET)
	{
		std::cout << "Failed to create listen socket.\n";
		return false;
	}
	std::cout << "Listen socket created successfully.\n";

	//Init server address with 0
	ZeroMemory((char*)&ServerAddress, sizeof(ServerAddress));

	//Set port number and server address values
	int port_number = port;
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_addr.s_addr = INADDR_ANY;
	ServerAddress.sin_port = htons(port_number);

	std::cout << "Server will run on port: " << port_number << "\n";

	//bind socket
	if (bind(ListenSocket, (sockaddr*)&ServerAddress, sizeof(ServerAddress)) == SOCKET_ERROR)
	{
		std::cout << "bind() failed.\n";
		return false;
	}
	std::cout << "Binded successfully.\n";

	//start listening 
	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cout << "listen() failed.\n";
		return false;
	}
	std::cout << "Started listening.\n";

	StartAccepting(ListenSocket);
	return true;
}

//Function that creates accept thread
bool StartAccepting(SOCKET ListenSocket)
{
	//Create accept event
	var::hAcceptEvent = WSACreateEvent();
	if (var::hAcceptEvent == WSA_INVALID_EVENT)
	{
		std::cout << "Error occured while executing WSACreateEvent().\n";
		return false;
	}
	std::cout << "Accept event created successfully.\n";

	//Set event role to accepting
	if (WSAEventSelect(ListenSocket, var::hAcceptEvent, FD_ACCEPT) == SOCKET_ERROR)
	{
		std::cout << "WSAEventSelect() failed.\n";
		return false;
	}
	std::cout << "WSAEventSelect() executed successfully.\n";

	//Create accept thread
	DWORD nThreadID;
	var::hAcceptThread = CreateThread(0, 0, AcceptThread, (void*)ListenSocket, 0, &nThreadID);

	//Sleep until key is pressed
	while (!_kbhit())
	{
		Sleep(0);
	}

	//Cleanup and close program.
	CleanUp();
	closesocket(ListenSocket);
	DeInitialize();
	return true;
}
#endif