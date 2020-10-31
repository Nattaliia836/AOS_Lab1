
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#pragma warning(disable : 6386 6385)

#include <iostream>
#include "string.h"
#include "winsock2.h"
#include "ws2tcpip.h"
#include <fstream>
#include <stdlib.h>
#include <sstream>
#include <chrono>
#include <ctime>  
#include <cstdlib>
using namespace std;


enum prevCommand
{
	Onetime,
	Auto
};


ofstream fout("log_client.txt");
prevCommand previousCommand;
SOCKET ConnectSocket;
bool isPasswordHacked = false;
int attempts = 0;

void getTime()
{
	auto end = std::chrono::system_clock::now();

	std::time_t end_time = std::chrono::system_clock::to_time_t(end);

	fout << std::ctime(&end_time) << ": ";
}
string convertToStr(char* cstr)
{
	string tmp(cstr);
	return tmp;
}

// get response after sending password command
void getResponseAfterPwdCommand()
{
	int bytesSent = 0;
	int bytesRecieved = SOCKET_ERROR;
	char* sendBuffer = new char[255];
	char* recieveBuffer = new char[255];

	if (previousCommand == Onetime)
	{
		// get command
		bytesRecieved = SOCKET_ERROR;
		while (bytesRecieved == SOCKET_ERROR)
		{
			bytesRecieved = recv(ConnectSocket, recieveBuffer, 255, 0);
		}

		string command = convertToStr(recieveBuffer);
		getTime(); fout << "got " << command << " from server" << endl;
		if (command == "hackMeHacked")
		{
			isPasswordHacked = true;

			// get amount of attempts
			bytesRecieved = SOCKET_ERROR;
			while (bytesRecieved == SOCKET_ERROR)
			{
				bytesRecieved = recv(ConnectSocket, recieveBuffer, 255, 0);
			}
			cout << "Password hacked! Password:" << recieveBuffer << endl;
		}
		else if (command == "hackMeTryAgain")
		{
			cout << "Wrong password! Attempts:" << ++attempts << endl;
		}
		else if (command == "hackMeBadServerResponse")
		{
			cout << "password wasn't parsed properly on server. Please, relaunch!" << endl;
		}
		else
		{
			cout << "Bad command recieved from server." << endl;
		}
	}
	else
	{
		// get command
		bytesRecieved = SOCKET_ERROR;
		while (bytesRecieved == SOCKET_ERROR)
		{
			bytesRecieved = recv(ConnectSocket, recieveBuffer, 255, 0);
		}

		string command = convertToStr(recieveBuffer);

		if (command == "hackMeHacked")
		{
			isPasswordHacked = true;

			// get amount of attempts
			bytesRecieved = SOCKET_ERROR;
			while (bytesRecieved == SOCKET_ERROR)
			{
				bytesRecieved = recv(ConnectSocket, recieveBuffer, 255, 0);
			}
			cout << "Password hacked! Password:" << recieveBuffer << endl;
		}
		else if (command == "hackMeTryAgain")
		{
			cout << "Password wasn't hacked." << endl;
		}
		else
		{
			cout << "Bad command recieved from server." << endl;
		}
	}
	delete[] sendBuffer;
	delete[] recieveBuffer;
}

// send one of the password commands to server
bool sendPasswordToServer(string command)
{

	int bytesSent = 0;
	int bytesRecieved = SOCKET_ERROR;
	char* sendBuffer = new char[255];
	char* recieveBuffer = new char[255];

	size_t isOnetime = command.find("hackMeByHand");
	size_t isAuto = command.find("hackMeAuto");
	if (isOnetime != string::npos)
	{
		previousCommand = Onetime;

		// send command
		bytesSent = send(ConnectSocket, command.c_str(), 255, 0);
		getTime(); fout << "sent " << command.c_str() << " to server" << endl;
		string args = "";
		cin >> args;
		bytesSent = send(ConnectSocket, args.c_str(), 255, 0);
		getTime(); fout << "sent " << args.c_str() << " to server" << endl;
		//cout << "Password sent: " << command.substr(20, command.length()) << endl;
	}
	else if (isAuto != string::npos)
	{
		previousCommand = Auto;
		// send command
		bytesSent = send(ConnectSocket, command.c_str(), 255, 0);
		getTime(); fout << "sent " << command.c_str() << " to server" << endl;
	}

	else
	{
		cout << "Bad command. Try again." << endl;
		return false;
	}
	delete[] sendBuffer;
	delete[] recieveBuffer;
	return true;
}

// get password length from server
int getPasswordLength()
{
	int bytesSent = 0;
	int bytesRecieved = SOCKET_ERROR;
	char* sendBuffer = new char[255];
	char* recieveBuffer = new char[255];
	int pwdLength = 0;

	// wait for length
	bytesRecieved = SOCKET_ERROR;
	while (bytesRecieved == SOCKET_ERROR)
	{
		bytesRecieved = recv(ConnectSocket, recieveBuffer, 255, 0);
		getTime(); fout << "got " << recieveBuffer << " from server" << endl;
	}

	stringstream ss(recieveBuffer);

	delete[] sendBuffer;
	delete[] recieveBuffer;
	// convert string to int and check whether it was succesful
	if (ss >> pwdLength)
	{
		return pwdLength;
	}
	else
	{
		return -1;
	}
}

// send first message to server
char* tryToStartMessaging()
{
	int bytesSent = 0;
	int bytesRecieved = SOCKET_ERROR;
	char* sendBuffer = new char[255];
	char* recieveBuffer = new char[255];

	cout << "type hackMeStart to start hacking!" << endl << "We also support who command!" << endl;
	// command that tries to get access to password cracker service
	cin >> sendBuffer;
	bytesSent = send(ConnectSocket, sendBuffer, 255, 0);
	getTime(); fout << "sent " << sendBuffer << " to server" << endl;
	// wait for response
	bytesRecieved = SOCKET_ERROR;
	while (bytesRecieved == SOCKET_ERROR)
	{
		bytesRecieved = recv(ConnectSocket, recieveBuffer, 255, 0);
		getTime(); fout << "got " << recieveBuffer << " from server" << endl;
	}
	delete[] sendBuffer;
	return recieveBuffer;
}


int initSocket() {
	//----------------------
	// Initialize Winsock 
	WSADATA wsaData; 
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
		printf("Error at WSAStartup()\n");

	//----------------------
	// Create a SOCKET for connecting to server
	ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port of the server to be connected to.
	sockaddr_in clientService;
	clientService.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &clientService.sin_addr.s_addr);
	clientService.sin_port = htons(1028);


	//----------------------
	// Connect to server.
	if (connect(ConnectSocket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
		printf("Failed to connect.\n");
		WSACleanup();
		return -1;
	}

	printf("Connected to server.\n");
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
	char* sendBuffer = new char[255];
	char* recieveBuffer = new char[255];

	string command = "";
	while (command != "hackMeOK")
	{
		command = convertToStr(tryToStartMessaging());
		if (command != "hackMeOK" && command != "Ponomarenko_Nataliia__K_24")
		{
			cout << "Wrong command to start using server! Try again." << endl;
		}
		if (command == "Ponomarenko_Nataliia__K_24")
		{
			cout << command << endl;
		}
	}

	cout << "Recieved hackMeOK from server" << endl;
	int pwdLength = getPasswordLength();

	if (pwdLength == -1)
	{
		cout << "Couldn't parse password length. Halting." << endl;
		return -1;
	}
	while (!isPasswordHacked)
	{
		if (command == "hackMeOK")
		{
			cout << "Server sent you the length of password " << pwdLength << endl << "Try to hack this, huh?!" << endl;
			while (!isPasswordHacked)
			{
				cout << "You have two options, Neo: " << endl << "hackMeAuto or hackMeByHand" << endl;
				string command;
				cin >> command;
				if (sendPasswordToServer(command))
				{
					getResponseAfterPwdCommand();
				}
			}
			cout << "Halting as password was cracked..." << endl;
		}
		else
		{
			cout << "Recieved " << command << "from server. Try again" << endl;
		}
	}
	delete[] sendBuffer;
	delete[] recieveBuffer;
	WSACleanup();
	system("pause");
	return 0;
}