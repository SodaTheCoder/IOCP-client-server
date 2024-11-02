/* IOCP-TLS SERVER -  Cleanup.h
* Author: Tadej S
* Description: This file contains
* functions to deinitialize all libraries
* and close program normally.
* Time of writing file: 1st November 2024*/

#ifndef CLEANUP_H
#define CLEANUP_H
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <iostream>
#include <string>
#include "var.h"

//Function that empties client list vector.
void CleanClientList()
{
	EnterCriticalSection(&var::csClientList);
	std::vector <Client*>::iterator IterClientContext;
	for (IterClientContext = var::client_vec.begin(); IterClientContext != var::client_vec.end(); IterClientContext++)
	{
		//IO will be cancelled and socket will be closed by Client class destructor.
		delete* IterClientContext;
	}
	var::client_vec.clear();
	LeaveCriticalSection(&var::csClientList);
}


/* Function that triggers shutdown events, shuts worker threads down
*  and cleans client list.*/
void CleanUp()
{
	//Trigger shutdown event
	SetEvent(var::hShutdownEvent);
	WaitForSingleObject(var::hAcceptThread, INFINITE);
	for (int i = 0; i < var::nThreads; i++)
	{
		//Help threads get out of blocking - GetQueuedCompletionStatus()
		PostQueuedCompletionStatus(var::hIOCompletionPort, 0, (DWORD)NULL, NULL);
	}

	WaitForMultipleObjects(var::nThreads, var::phWorkerThreads, TRUE, INFINITE);
	WSACloseEvent(var::hAcceptEvent);
	CleanClientList();
}

/* Functions that deletes critical sections, closes handles and
*  performs WinSock library cleanup.*/
void DeInitialize()
{
	DeleteCriticalSection(&var::csConsole);
	DeleteCriticalSection(&var::csClientList);
	CloseHandle(var::hIOCompletionPort);
	CloseHandle(var::hShutdownEvent);
	delete[] var::phWorkerThreads;
	WSACleanup();
}

#endif