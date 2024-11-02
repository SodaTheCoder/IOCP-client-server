/* IOCP-TLS SERVER -  Worker_thread.h
* Author: Tadej S
* Description: This file contains Worker
* thread function that fulfills client
* requests.
* Time of writing file: 1st November 2024*/

#ifndef WORKER_THREAD_H
#define WORKER_THREAD_H
#include <winsock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include "var.h"
#include "client.h"

//Function that fulfills client requests. Main function of program.
DWORD WINAPI WorkerThread(LPVOID data)
{
	//Create variables that function needs to use
	int thread_number = (int)data;
	void* pContext = NULL;
	OVERLAPPED* lpOverlapped = NULL;
	Client* pClient = NULL;

	DWORD dwBytesTransfered = 0, dwBytes = 0, dwFlags = 0;
	int nBytesSent = 0;
	int nBytesRecv = 0;
	
	//Loop until shutdown event is not signaled
	while (WaitForSingleObject(var::hShutdownEvent, 0) != WAIT_OBJECT_0)
	{
		//Attempts to dequeue an I/O completion packet from the specified I/O completion port
		BOOL bReturn = GetQueuedCompletionStatus(var::hIOCompletionPort,
			&dwBytesTransfered,
			(PULONG_PTR)&pContext,
			&lpOverlapped,
			INFINITE);
		
		if (pContext == NULL)
		{
			tprint("pContext was NULL, terminating...\n");
			break;
		}

		pClient = (Client*)pContext;
		//Client disconnected, so remove it from vector.
		if (bReturn == FALSE || bReturn == TRUE && dwBytesTransfered == 0)
		{
			RemoveFromClientListAndFreeMemory(pClient);
		}

		WSABUF* p_wbuf = pClient->GetWSABUFPtr();
		OVERLAPPED* p_ol = pClient->GetOLPtr();

		//Determine if client wants us to receive or send data.
		switch (pClient->GetOpCode())
		{
		case OP_READ:
			//Client wants us to read data
			pClient->IncrSentBytes(dwBytesTransfered);

			//Write operation was finished, see if all the data was sent. If not, post another write.
			if (pClient->GetSentBytes() < pClient->GetTotalBytes())
			{
				pClient->SetOpCode(OP_READ);
				p_wbuf->buf += pClient->GetSentBytes();
				p_wbuf->len = pClient->GetTotalBytes() - pClient->GetSentBytes();
				dwFlags = 0;

				nBytesSent = WSASend(pClient->GetSocket(), p_wbuf, 1,
					&dwBytes, dwFlags, p_ol, NULL);

				if ((nBytesSent == SOCKET_ERROR) && (WSAGetLastError() != WSA_IO_PENDING))
				{
					RemoveFromClientListAndFreeMemory(pClient);
				}
			}

			else
			{
				//Receive data from client
				pClient->SetOpCode(OP_WRITE);
				pClient->ResetWSABUF();
				dwFlags = 0;

				nBytesRecv = WSARecv(pClient->GetSocket(), p_wbuf, 1, &dwBytes, &dwFlags, p_ol, NULL);

				if ((nBytesRecv == SOCKET_ERROR) && (WSAGetLastError() != WSA_IO_PENDING))
				{
					tprint("Thread %d: Error has occured while executing WSARecv().\n", thread_number);
					RemoveFromClientListAndFreeMemory(pClient);
				}
			}
			break;
		case OP_WRITE:
			//Client wants us to send data
			//Print received message
			char szBuffer[MAX_BUFFER_LEN];
			pClient->GetBuffer(szBuffer);
			tprint("Thread %d: Following message was received: %s", thread_number, szBuffer);

			pClient->SetOpCode(OP_READ);
			pClient->SetTotalBytes(dwBytesTransfered);
			pClient->SetSentBytes(0);
			p_wbuf->len = dwBytesTransfered;
			dwFlags = 0;

			nBytesSent = WSASend(pClient->GetSocket(), p_wbuf, 1, &dwBytes, dwFlags, p_ol, NULL);
			if ((nBytesSent == SOCKET_ERROR) && (WSAGetLastError() != WSA_IO_PENDING))
			{
				tprint("Thread %d: Error has occured while executing WSASend().\n", thread_number);
				RemoveFromClientListAndFreeMemory(pClient);
			}
			break;
		default:
			//Program should never reach that point under normal circumstances.
			break;
		}
	}
	return 0;
}

#endif