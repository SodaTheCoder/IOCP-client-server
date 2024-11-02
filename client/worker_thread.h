/* IOCP-TLS CLIENT -  Worker_thread.h
* Author: Tadej S
* Description: This file contains Worker
* thread function that fulfills client
* requests.
* Time of writing file: 2nd November 2024*/

#ifndef WORKER_THREAD_H
#define WORKER_THREAD_H
#include <winsock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
#include "var.h"

DWORD WINAPI WorkerThread(LPVOID data)
{
	ThreadInfo* pThreadInfo = (ThreadInfo*)data;
	char szTemp[MAX_BUFFER_LEN];
	int nBytesSent = 0;
	int nBytesRecv = 0;

	for (int i = 0; i < pThreadInfo->nNoOfSends; i++)
	{
		//Send message to server
		sprintf(szTemp, "%d. %s", i + 1, pThreadInfo->m_szBuffer);
		nBytesSent = send(pThreadInfo->m_Socket, szTemp, strlen(szTemp), 0);

		if (nBytesSent == SOCKET_ERROR)
		{
			tprint("Error occurred while writing to socket %ld.\n", WSAGetLastError());
			return 1;
		}

		//Receive message from server
		nBytesRecv = recv(pThreadInfo->m_Socket, szTemp, 255, 0);

		if (nBytesRecv == ERROR)
		{
			tprint("Error occurred while reading from socket %ld.\n", WSAGetLastError());
			return 1;
		}

		//Display the server message
		tprint("Server sent the following message: %s\n", szTemp);
	}

	return 0;
}

#endif
