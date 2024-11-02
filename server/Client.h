#ifndef CLIENT_H
#define CLIENT_H
#include <winsock2.h>
/* IOCP-TLS SERVER -  Client.h
* Author: Tadej S
* Description: This file contains
* Client class and function to add client
* object to a vector or to remove it from it.
* Time of writing file: 1st November 2024*/

#include <WS2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include "var.h"

/* Class that contains data about client,
*  such as Socket, received buffer, and
*  OVERLAPPED pointer for I/O completion port.*/
class Client
{
private:
	//Client dependent variables
	OVERLAPPED* m_pol;
	WSABUF* m_pwbuf;
	int               m_nTotalBytes;
	int               m_nSentBytes;
	SOCKET            m_Socket;
	int               m_nOpCode;
	char              m_szBuffer[MAX_BUFFER_LEN];

public:
	//Public get - set functions
	void    SetOpCode(int n) { m_nOpCode = n; }
	int     GetOpCode() { return m_nOpCode; }
	void    SetTotalBytes(int n) { m_nTotalBytes = n; }
	int     GetTotalBytes() { return m_nTotalBytes; }
	void    SetSentBytes(int n) { m_nSentBytes = n; }
	void    IncrSentBytes(int n) { m_nSentBytes += n; }
	int     GetSentBytes() { return m_nSentBytes; }
	void    SetSocket(SOCKET s) { m_Socket = s; }
	SOCKET  GetSocket() { return m_Socket; }
	void    SetBuffer(char* buf) { strcpy(m_szBuffer, buf); }
	void    GetBuffer(char* buf) { strcpy(buf, m_szBuffer); }
	void    ZeroBuffer() { ZeroMemory(m_szBuffer, MAX_BUFFER_LEN); }
	void    SetWSABUFLength(int nLen) { m_pwbuf->len = nLen; }
	int     GetWSABUFLength() { return m_pwbuf->len; }
	WSABUF* GetWSABUFPtr() { return m_pwbuf; }
	OVERLAPPED* GetOLPtr() { return m_pol; }

	void ResetWSABUF()
	{
		ZeroBuffer();
		m_pwbuf->buf = m_szBuffer;
		m_pwbuf->len = MAX_BUFFER_LEN;
	}

	//Constructor - allocate memory for variables
	Client()
	{
		//Allocate memory
		m_pol = new OVERLAPPED;
		std::cout << "OVERLAPPED object dynamically allocated successfully.\n";
		m_pwbuf = new WSABUF;
		std::cout << "WSABUF object dynamically allocated successfully.\n";

		//Initialize pointers with zero
		ZeroMemory(m_pol, sizeof(OVERLAPPED));
		std::cout << "ZeroMemory() for OVERLAPPED object executed successfully.\n";
		m_Socket = SOCKET_ERROR;

		ZeroMemory(m_szBuffer, MAX_BUFFER_LEN);
		std::cout << "ZeroMemory() for WSABUF object executed successfully.\n";

		//Set default values to variables
		m_pwbuf->buf = m_szBuffer;
		m_pwbuf->len = MAX_BUFFER_LEN;
		m_nOpCode = 0;
		m_nTotalBytes = 0;
		m_nSentBytes = 0;
		std::cout << "Set variables to default values successfully.\n";
	}

	//Destructor - deallocate memory of variables
	~Client()
	{
		//Wait for I/O operations to complete
		while (!HasOverlappedIoCompleted(m_pol))
		{
			Sleep(0);
		}

		//close socket and deallocate memory
		closesocket(m_Socket);
		delete m_pol;
		delete m_pwbuf;
	}
};

//Funtion that adds accepted client to client vector.
void AddToClientList(Client* pClient)
{
	EnterCriticalSection(&var::csClientList);
	var::client_vec.push_back(pClient);
	LeaveCriticalSection(&var::csClientList);
}

//Function that removes single client from client vector.
void RemoveFromClientListAndFreeMemory(Client* pClient)
{
	EnterCriticalSection(&var::csClientList);
	std::vector <Client*>::iterator IterClientContext;

	//Remove the supplied Client* from the list and release the memory
	for (IterClientContext = var::client_vec.begin(); IterClientContext != var::client_vec.end(); IterClientContext++)
	{
		if (pClient== *IterClientContext)
		{
			var::client_vec.erase(IterClientContext);
			delete pClient;
			break;
		}
	}

	LeaveCriticalSection(&var::csClientList);
}

#endif