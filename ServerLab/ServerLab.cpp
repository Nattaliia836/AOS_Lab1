#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include "winsock2.h"
#include "ws2tcpip.h"
#include <stdlib.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <ctime>  

using namespace std;

int attempts = 0;
char* password = new char[255];

ofstream fout("logFileForServer.txt");
SOCKET ListenSocket;
bool isPasswordHacked = false;
vector<string> alphabet = { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
							"0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
							"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z" };


void getTime()
{
	auto end = std::chrono::system_clock::now();

	std::time_t end_time = std::chrono::system_clock::to_time_t(end);

	fout << std::ctime(&end_time) << ": ";
}

void recursiveGenerator(string prefix, int lengthLeft)
{
	if (isPasswordHacked)
	{
		return;
	}
	else  if (lengthLeft == 0)
	{
		string attemptedPasswordString = prefix;

		if (strcmp(attemptedPasswordString.c_str(), password) == 0)
		{
			isPasswordHacked = true;
			cout << "Pwd hacked! Pwd itself:" << attemptedPasswordString << endl;
		}
		else
		{
			attempts++;
		}
		std::cout << "Password:" << attemptedPasswordString << " , is hacked:" << isPasswordHacked << endl;
	}
	else
	{
		for (size_t i = 0; i < alphabet.size(); i++)
		{
			recursiveGenerator(prefix + alphabet[i], lengthLeft - 1);
		}
	}
}

void hackPwd(int length)
{
	recursiveGenerator("", length);
}

void sendGotItAndPassword()
{
	int bytesSent = 0;
	int bytesRecieved = SOCKET_ERROR;
	const char* sendBuffer = new char[255];
	char* recieveBuffer = new char[255];

	stringstream ss;
	ss << attempts;

	string arg = ss.str();
	string gotIt = "hackMeHacked";

	sendBuffer = gotIt.c_str();
	// отправить комманду что пароль взломан
	bytesSent = send(ListenSocket, sendBuffer, 255, 0);
	getTime(); fout << "send " << sendBuffer << " to client" << endl;

	// отправить пароль
	sendBuffer = password;
	bytesSent = send(ListenSocket, sendBuffer, 255, 0);
	getTime(); fout << "send " << sendBuffer << " to client" << endl;

}

char* getArgument()
{
	int bytesSent = 0;
	int bytesRecieved = SOCKET_ERROR;
	const char* sendBuffer = new char[255];
	char* recieveBuffer = new char[255];

	bytesRecieved = SOCKET_ERROR;
	while (bytesRecieved == SOCKET_ERROR)
	{
		bytesRecieved = recv(ListenSocket, recieveBuffer, 255, 0);
		getTime(); fout << "get " << recieveBuffer << " from client" << endl;
	}
	return recieveBuffer;
}

string getPasswordCommand()
{

	int bytesSent = 0;
	int bytesRecieved = SOCKET_ERROR;
	const char* sendBuffer = new char[255];
	char* recieveBuffer = new char[255];

	bytesRecieved = SOCKET_ERROR;
	while (bytesRecieved == SOCKET_ERROR)
	{
		bytesRecieved = recv(ListenSocket, recieveBuffer, 255, 0);
		getTime(); fout << "get " << recieveBuffer << " from client" << endl;
	}


	string command(recieveBuffer);
	return command;
}

void loopUntilPwdHacked()
{
	int bytesSent = 0;
	int bytesRecieved = SOCKET_ERROR;
	const char* sendBuffer = new char[255];
	char* recieveBuffer = new char[255];

	while (!isPasswordHacked)
	{

		string onetimeToken = "hackMeByHand";
		string autoToken = "hackMeAuto";

		string command = getPasswordCommand();
		cout << "Got this pwd command from client:" << command << endl;
		if (command == onetimeToken)
		{
			char* arg = getArgument();

			if (strcmp(arg, password) == 0)
			{
				sendGotItAndPassword();
				getTime(); fout << "send " << sendBuffer << " to client" << endl;
				isPasswordHacked = true;
			}
			else
			{
				sendBuffer = "hackMeTryAgain";
				bytesSent = send(ListenSocket, sendBuffer, 255, 0);
				getTime(); fout << "send " << sendBuffer << " to client" << endl;
			}

			cout << "Password:" << arg << " , is hacked:" << isPasswordHacked << endl;
		}
		else if (command == autoToken)
		{
			attempts = 0;
			hackPwd(strlen(password));
			if (isPasswordHacked)
			{
				sendGotItAndPassword();
			}
			else
			{
				sendBuffer = "hackMeTryAgain";
				bytesSent = send(ListenSocket, sendBuffer, 255, 0);
				getTime(); fout << "send " << sendBuffer << " to client" << endl;
			}

		}
	}
	// delete[] sendBuffer;
	// delete[] recieveBuffer;
}

void sendPasswordLength()
{
	int bytesSent = 0;
	// convert it to c-string
	int pwdLength = strlen(password);
	string pwdLengthStr = to_string(pwdLength);
	const char* pwdLengthCStr = pwdLengthStr.c_str();

	// send password length
	bytesSent = send(ListenSocket, pwdLengthCStr, 255, 0);
	getTime(); fout << "send " << pwdLengthCStr << " to client" << endl;

}

int sendOKToClient()
{
	int bytesSent = 0;
	int bytesRecieved = SOCKET_ERROR;
	const char* sendBuffer = new char[255];
	char* recieveBuffer = new char[255];


	bool sentOK = false;

	while (!sentOK)
	{
		bytesRecieved = SOCKET_ERROR;
		while (bytesRecieved == SOCKET_ERROR)
		{
			bytesRecieved = recv(ListenSocket, recieveBuffer, 255, 0);
			getTime(); fout << "get " << recieveBuffer << " from client" << endl;
		}
		string command(recieveBuffer);
		cout << command << endl;
		if (command == "hackMeStart")
		{
			sendBuffer = "hackMeOK";
			bytesSent = send(ListenSocket, sendBuffer, 255, 0);
			getTime(); fout << "send " << sendBuffer << " to client" << endl;
			cout << "sent hackMeOK to client" << endl;
			sentOK = true;
		}
		else if (command == "who")
		{
			sendBuffer = "Ponomarenko_Nataliia__K_24";
			bytesSent = send(ListenSocket, sendBuffer, 255, 0);
			getTime(); fout << "send " << sendBuffer << " to client" << endl;
			cout << "sent who to client" << endl;
		}
		else
		{
			sendBuffer = "hackMeBadCmd";
			cout << "sent hackMeBadCmd to client" << endl;
			bytesSent = send(ListenSocket, sendBuffer, 255, 0);
			getTime(); fout << "send " << sendBuffer << " to client" << endl;
		}
	}
	return 0;
}

int initSocket()
{
	//----------------------
	// Initialize Winsock
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
		printf("Error at WSAStartup()\n");

	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port for the socket that is being bound.
	sockaddr_in service;
	service.sin_family = PF_INET;
	inet_pton(AF_INET, "127.0.0.1", &service.sin_addr.s_addr);
	service.sin_port = htons(1028);

	//----------------------
	// Bind the socket.
	if (::bind(ListenSocket,
		(SOCKADDR*)&service,
		sizeof(service)) == SOCKET_ERROR) {
		printf("bind() failed.\n");
		closesocket(ListenSocket);
		return -1;
	}

	//----------------------
	// Listen for incoming connection requests 
	// on the created socket
	if (listen(ListenSocket, 1) == SOCKET_ERROR)  //можливе лише одне підключення
		printf("Error listening on socket.\n");

	printf("Listening on socket...\n");

	SOCKET AcceptSocket;
	printf("Waiting for client to connect...\n");

	// Accept the connection.
	while (1) {
		AcceptSocket = SOCKET_ERROR;
		while (AcceptSocket == SOCKET_ERROR) {
			AcceptSocket = accept(ListenSocket, NULL, NULL);
		}
		printf("Client connected.\n");
		ListenSocket = AcceptSocket;
		break;
	}
	return 0;
}



int main()
{

	if (initSocket() == -1)
	{
		return -1;
	}

	int bytesSent = 0;
	int bytesRecieved = SOCKET_ERROR;
	const char* sendBuffer = new char[255];
	char* recieveBuffer = new char[255];

	sendOKToClient();

	cout << "Type password (max length = 255, use lower & uppercase letters + numbers):" << endl;

	// check for correctness of input
	if (cin >> password)
	{
		sendPasswordLength();
		loopUntilPwdHacked();
		return 0;
	}
	else
	{
		sendBuffer = "hackMeBadServerResponse";
		bytesSent = send(ListenSocket, sendBuffer, 255, 0);
		getTime(); fout << "send " << sendBuffer << " to client" << endl;
		// delete[] sendBuffer;
		// delete[] recieveBuffer;
		WSACleanup();
		return -1;
	}

}