/* IOCP-TLS CLIENT -  var.h
* Author: Tadej S
* Description: This file contains
* namespace of global variables that are
* used in the program. It also
* contains tprint function.
* Time of writing file: 2nd November 2024*/

#ifndef VAR_H
#define VAR_H
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <iostream>
#include <string>
#define MAX_BUFFER_LEN 256

//Structure to pass information to Thread
struct ThreadInfo
{
	int nThreadNo;
	int nNoOfSends;
	SOCKET m_Socket;
	char m_szBuffer[MAX_BUFFER_LEN];
};

namespace var
{
	CRITICAL_SECTION csConsole;
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