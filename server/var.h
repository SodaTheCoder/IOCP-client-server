/* IOCP-TLS SERVER -  var.h
* Author: Tadej S
* Description: This file contains
* namespace of global variables that are 
* used in the program. It also 
* contains tprint function.
* Time of writing file: 1st November 2024*/

#ifndef VAR_H
#define VAR_H
#include <windows.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <vector>
#define WAIT_TIMEOUT_INTERVAL 100
#define MAX_BUFFER_LEN 256
#define OP_READ 0
#define OP_WRITE 1

class Client;

namespace var
{
	int nThreads = 0;
	HANDLE* phWorkerThreads = NULL;

	CRITICAL_SECTION csConsole;
	CRITICAL_SECTION csClientList;

	HANDLE hShutdownEvent = NULL;

	HANDLE hIOCompletionPort = NULL;

	WSAEVENT hAcceptEvent;
	HANDLE hAcceptThread = NULL;

	std::vector<Client*> client_vec;
}

/* Function that prints string to console.
*  printf() is not thread-safe, so calling it
*  from multiple threads at the same time can
*  result in undefined behaviour. This function
*  uses critical section to make it thread-safe.*/
void tprint(const char* szFormat, ...)
{
	EnterCriticalSection(&var::csConsole);

	va_list args;
	va_start(args, szFormat);
	vprintf(szFormat, args);
	va_end(args);

	LeaveCriticalSection(&var::csConsole);
}

#endif