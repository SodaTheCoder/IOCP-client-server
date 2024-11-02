/* IOCP-TLS CLIENT - ClientIOCP.cpp
* Author: Tadej S
* Description: This file contains main
* function definition.
* Time of writing file: 2nd November 2024*/

#define WIN32_LEAN_AND_MEAN
#include <openssl/applink.c>
#include "StdAfx.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include "var.h"
#include "create_socket.h"
#include "worker_thread.h"

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		std::cout << "Usage: " << argv[0] << " hostname port.\n";
		return 1;
	}

	//Initialize WinSock
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (iResult != NO_ERROR)
	{
		std::cout << "Error occured while executing WSAStartup().\n";
		return 1;
	}

	InitializeCriticalSection(&var::csConsole);
	int nPortNo = atoi(argv[2]);

	char szBuffer[MAX_BUFFER_LEN];
	gets_s(szBuffer);
	int nNoOfThreads = 1;
	int nNoOfSends = 1;
	printf("enter number of threads:");
	scanf("%d", &nNoOfThreads);
	printf("\nenter number of sends per thread:");
	scanf("%d", &nNoOfSends);

	HANDLE* hThreads = new HANDLE[nNoOfThreads];
	ThreadInfo* pThreadInfo = new ThreadInfo[nNoOfThreads];

	bool SocketCreated = false;
	DWORD nThreadID;

	for (int i = 0; i < nNoOfThreads; i++)
	{
		SocketCreated = CreateConnectedSocket(&(pThreadInfo[i].m_Socket), argv[1], nPortNo);
		//socket creation has failed, let's terminate
		if (!SocketCreated)
		{
			std::cout << "Socket creation failed.\n";
			delete[] hThreads;
			delete[] pThreadInfo;
			return 1;
		}

		pThreadInfo[i].nNoOfSends = nNoOfSends;
		pThreadInfo[i].nThreadNo = i + 1;
		sprintf(pThreadInfo[i].m_szBuffer, "Thread %d - %s", i + 1, szBuffer);

		hThreads[i] = CreateThread(0, 0, WorkerThread, (void*)(&pThreadInfo[i]), 0, &nThreadID);
	}

	/*Wait for worker threads to complete, then
	do cleanup and terminate program*/
	WaitForMultipleObjects(nNoOfThreads, hThreads, TRUE, INFINITE);
	for (int ii = 0; ii < nNoOfThreads; ii++)
	{
		closesocket(pThreadInfo[ii].m_Socket);
	}

	delete[] hThreads;
	delete[] pThreadInfo;
	DeleteCriticalSection(&var::csConsole);
	WSACleanup();
	return 0;
}