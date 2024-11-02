/* IOCP-TLS SERVER -  Main.cpp
* Author: Tadej S
* Description: This file contains
* main function definition.
* Time of writing file: 1st November 2024*/

#define WIN32_LEAN_AND_MEAN
#include "StdAfx.h"
#include <openssl/applink.c>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <vector>
#include "Initialize.h"

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cout << "Usage: " << argv[0] << " port.\n";
		return 1;
	}

	std::cout << "TCLOUD SERVER. (C)2024, TadejS. Time: " << __TIME__ << "\n";
	if (!Initialize())
	{
		std::cout << "Failed to initialize.\n";
	}

	int port = atoi(argv[1]);
	if (!CreateSocket(port))
	{
		std::cout << "Error occured in CreateSocket() function.\n";
	}
	return 0;
}