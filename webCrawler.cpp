// webCrawler.cpp : Defines the entry point for the console application.
//

#pragma comment(lib, "ws2_32")
#include "stdafx.h"
#include <winsock2.h>
#include <stdio.h>

#include <iostream>
#include <string>
#include <algorithm>
#include <vector>

#include <curl/curl.h>
using namespace std;

#define limit 99

string hotmal[limit+1];//gotta do these here to make sure that the search function can also see and modify these variables.
string link;
int queuesize = 0;


//~~maybe this oughta be the client side...it only ever sends, never recieves.~~ Server. It has to be server. Because database is server, and as such indexer is client.
//Except...for the part I can't run two servers at the same time? But maybe I *can,* on different ports.
//Perhaps it oughta send on the whole text it pulls from the internet so that the indexer can search through it without needing curl? Yeah, I'll do that.

//int 
void findAllOccurances(vector<size_t> & vec, string data, string toSearch)
{
	// Get the first occurrence
	size_t pos = data.find(toSearch);
	// Repeat till end is reached
	while (pos != std::string::npos)
	{
		// Add position to the vector
		vec.push_back(pos);
		// Get the next occurrence from the current position
		pos = data.find(toSearch, pos + toSearch.size());
	}
}


static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	((std::string*)userp)->append((char*)contents, realsize);

	return realsize;
}

int linksearch(string found) {
	int total = 0;

	CURL *curl;
	CURLcode res;

	std::string reslink;

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, found.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &reslink);

		res = curl_easy_perform(curl);
		/* Check for errors */
		if (res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}
		else {
			/*the string we're looking for is    <a href="http
			//and we need to locate every instance
			//and record it from    http   up to   "
			//so! perhaps I need a new function that searches up to every *new* http tag (takes the string recieved, and total, as values and otherwise starts from the top)
			//and returns the value location of   <a href="http     +9.                    or I could use .find()         (axe)
			//perhaps then I need some other function to start from there until it finds a  "  that occurs later in...    (why)
			//string foundlink = reslink.substr (axe, why-axe);
			//or maybe I'm making this too complex on myself.
			//in any case, I need to extract that  res  value into a string.*/

			vector<size_t> linkLocations;
			findAllOccurances(linkLocations, reslink, "<a href=\"http");

			cout << found << "         ";
			//cout << linkLocations;

			while (queuesize + total < limit + 1) {
				//search for every link [or, actually, have already found them up here and put them in a list] kinda
				if (total< linkLocations.size()) {
					int axe = linkLocations[total] + 9;
					int why = reslink.find("\"", axe);//
					hotmal[queuesize + total] = reslink.substr(axe, why - axe);
					total++;
				}
				else { break; }
			}

			/* always cleanup */
			curl_easy_cleanup(curl);
		}
	}
	return total;
}

int main(int argc, char **argv)
{
	WSADATA              wsaData;

	// Initialize Winsock version 2.2
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	printf("Winsock DLL status is %s.\n", wsaData.szSystemStatus);

	// Code to handle socket:
	SOCKET Socket;

	// Create a new socket to make a client or server connection.
	// AF_INET = 2, The Internet Protocol version 4 (IPv4) address family, TCP protocol
	Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == INVALID_SOCKET)
	{
		printf("Socket() failed!Error code : %ld\n", WSAGetLastError());
		// Do the clean up
		WSACleanup();
		// Exit with error
		return -1;
	}

	//Socket address structure
	SOCKADDR_IN          ServerAddr;
	unsigned int         Port = 21002;

	// IPv4
	ServerAddr.sin_family = AF_INET;
	// Port no.
	ServerAddr.sin_port = htons(Port);
	// The IP address
	ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);


	////Bind to socket
	// Associate the address information with the socket using bind.
	// Call the bind function, passing the created socket and the sockaddr_in
	// structure as parameters. Check for general errors.
	if (bind(Socket, (SOCKADDR *)&ServerAddr, sizeof(ServerAddr)) == SOCKET_ERROR)
	{
		printf("Server: bind() failed! Error code: %ld.\n", WSAGetLastError());
		// Close the socket
		closesocket(Socket);
		// Do the clean up
		WSACleanup();
		// and exit with error
		return -1;
	}
	else
		printf("Server: bind() is OK!\n");


	////Listen to Socket for client
	// Listen for client connections. We use a backlog of 5, which
	// is normal for many applications.
	if (listen(Socket, 5) == SOCKET_ERROR)
	{
		printf("Server: listen(): Error listening on socket %ld.\n", WSAGetLastError());
		// Close the socket
		closesocket(Socket);
		// Do the clean up
		WSACleanup();
		// Exit with error
		return -1;
	}
	else
		printf("Server: listen() is OK!\n");


	SOCKET NewConnection;
	bool keepconnected = true;
	while (keepconnected == true) {

		NewConnection = accept(Socket, NULL, NULL);
		if (NewConnection == SOCKET_ERROR) {
			printf("Server: accept() failed!Error code : %ld\n", WSAGetLastError());
			// Close the socket
			closesocket(Socket);
			// Do the clean up
			WSACleanup();
			// Exit with error
			return -1;
		}
		else
			printf("Server: accept() is OK!\n");






		string youAreEl;

		/*1. Begin with a base URL that you select, and place it on the top of your queue
		2. Pop the URL at the top of the queue and download it
		3. Parse the downloaded HTML file and extract all links
		4. Insert each extracted link into the queue
		5. Goto step 2, or stop once you reach some specified limit*/

		bool really = false;
		while (really == false) {
			string someMore;
			while (someMore != "no") {
				cout << "Enter base URL ";
				cin >> youAreEl;//getline(cin, youAreEl) works too

				hotmal[queuesize] = youAreEl;
				queuesize++;

				cout << "Input another URL? If not, type \"no\". ";
				cin >> someMore;
			}
			if (hotmal[0].empty()) {
				cout << "Nice try, bucko.\n";
			}
			else { really = true; }
		}


		//this loop is actually doing the crawling
		while (hotmal[limit].empty()) {
			//search link

			link = hotmal[0];//does this make a pointer? I sure hope it doesn't

			queuesize = queuesize + linksearch(link);


			//now send it!
			//format: GIB length-of-link link the-whole-downloaded-thing
			/*int nope = 0;
			while (nope == 0) {
				char recvbuf[4000];
				memset(recvbuf, 0, 4000);
				//char* recvbuf = &recieverbuffer[0];

				int BytesReceived = recv(NewConnection, recvbuf, sizeof(recvbuf), 0);
				if (BytesReceived == SOCKET_ERROR) {
					printf("Client: recv() error %ld.\n", WSAGetLastError());
					nope = 1;
				}

				char wordswordswords[4050];
				strcpy(wordswordswords, "GIB ");
				strcat(wordswordswords, link.size());
				strcat(wordswordswords, " ");
				strcat(wordswordswords, link);
				strcat(wordswordswords, " ");
				strcat(wordswordswords, downloaded-thing);//maybe I should introduce a cstring before all of the everything that contains a copy of the reslink
				strcat(wordswordswords, "\r\n"); //shoot, I copied RECIEVE code not immediately SEND code. Check myself before I do it wrong...server code tried to...
			//actually, it needs a notif from client before it sends it anyways, thanks to client-indexer needing to ask for this due to switching.
			//actually no...client gets, first! in how I currently have it.


				char senderbuffer[4000];
				memset(senderbuffer, 0, 4000);
				const char* sendbuf = &senderbuffer[0];

				strcpy(senderbuffer, wordswordswords);
				int BytesSent = send(NewConnection, sendbuf, strlen(sendbuf), 0);
				//printf("%s %d %d\n", senderbuffer, strlen(sendbuf), BytesSent);
				if (BytesSent == SOCKET_ERROR) {
					printf("Client: send() error %ld.\n", WSAGetLastError());
					nope = 1;
				}

			}*/



			//if (hotmal[limit].empty()) {
			int copy = 0;
			while (!hotmal[copy].empty()) {
				hotmal[copy] = hotmal[copy + 1];
				copy++;
			}
			//}
		}








		/*int nope = 0;
		while (nope == 0) {
			char recvbuf[4000];
			memset(recvbuf, 0, 4000);
			//char* recvbuf = &recieverbuffer[0];

			int BytesReceived = recv(NewConnection, recvbuf, sizeof(recvbuf), 0);
			if (BytesReceived == SOCKET_ERROR) {
				printf("Client: recv() error %ld.\n", WSAGetLastError());
				nope = 1;
			}

			char wordswordswords[4050];
			//I need an string array to put quotes gotten into depending on 
			if (recvbuf[0] == 'S'&&recvbuf[1] == 'E'&&recvbuf[2] == 'T') {
				int whichquote;
				if (recvbuf[4] == '0' || recvbuf[4] == '1' || recvbuf[4] == '2' || recvbuf[4] == '3' || recvbuf[4] == '4' || recvbuf[4] == '5' || recvbuf[4] == '6' || recvbuf[4] == '7' || recvbuf[4] == '8' || recvbuf[4] == '9') {
					if (recvbuf[5] == ' ') {
						whichquote = atoi(recvbuf + 4) - 1;
						quotes[whichquote][0] = recvbuf + 6;

						strcpy(wordswordswords, "OK ");
						strcat(wordswordswords, recvbuf + 4);
						strcat(wordswordswords, "\r\n");
					}
					else {
						whichquote = atoi(recvbuf + 4) - 1;

						if (whichquote > 19) {
							break;
						}
						else {
							quotes[whichquote][0] = recvbuf + 7;

							strcpy(wordswordswords, "OK ");
							strcat(wordswordswords, recvbuf + 4);
							strcat(wordswordswords, "\r\n");
						}
					}
				}
				else {
					strcpy(wordswordswords, "ERR BAD REQUEST\r\n");
				}
			}
			else if (recvbuf[0] == 'G'&&recvbuf[1] == 'E'&&recvbuf[2] == 'T') {
				if (recvbuf[4] == '0' || recvbuf[4] == '1' || recvbuf[4] == '2' || recvbuf[4] == '3' || recvbuf[4] == '4' || recvbuf[4] == '5' || recvbuf[4] == '6' || recvbuf[4] == '7' || recvbuf[4] == '8' || recvbuf[4] == '9') {
					int whichquote;
					if (recvbuf[5] == ' ') {
						whichquote = atoi(recvbuf + 4) - 1;

						strcpy(wordswordswords, "OK ");
						strcat(wordswordswords, recvbuf + 4);
						strcat(wordswordswords, " ");
						strcat(wordswordswords, quotes[whichquote][0]);
						strcat(wordswordswords, "\r\n");
					}
					else {
						whichquote = atoi(recvbuf + 4) - 1;
						if (whichquote > 19) {
							break;
						}
						else {
							strcpy(wordswordswords, "OK ");
							strcat(wordswordswords, recvbuf + 4);
							strcat(wordswordswords, " ");
							strcat(wordswordswords, quotes[whichquote][0]);
							strcat(wordswordswords, "\r\n");
						}
					}
				}
				else {
					strcpy(wordswordswords, "ERR BAD REQUEST\r\n");
				}
			}

			char senderbuffer[4000];
			memset(senderbuffer, 0, 4000);
			const char* sendbuf = &senderbuffer[0];

			strcpy(senderbuffer, wordswordswords);
			int BytesSent = send(NewConnection, sendbuf, strlen(sendbuf), 0);
			//printf("%s %d %d\n", senderbuffer, strlen(sendbuf), BytesSent);
			if (BytesSent == SOCKET_ERROR) {
				printf("Client: send() error %ld.\n", WSAGetLastError());
				nope = 1;
			}

		}*/











		if (keepconnected == true)
		{
			keepconnected = true;
		}

	}

	////shutdown connection when done
	// Shutdown sending of data
	if (shutdown(Socket, SD_SEND) != 0)
	{
		printf("Client: Well, there is something wrong with the shutdown(). The error code: %ld\n", WSAGetLastError());
	}
	else
	{
		printf("Client: shutdown() looks OK...\n\n");
	}

	////close socket
	if (closesocket(Socket) != 0)
	{
		printf("Server: Cannot close socket. Error code: %ld\n", WSAGetLastError());
	}
	else
	{
		printf("Server: Closing socket...\n");
	}

	////call cleanup when no more sockets remain
	if (WSACleanup() != 0)
	{
		printf("Client: WSACleanup() failed!...\n");
	}



	// Cleanup socket
	WSACleanup();
}


void main()
{

	string youAreEl;

	/*1. Begin with a base URL that you select, and place it on the top of your queue
2. Pop the URL at the top of the queue and download it
3. Parse the downloaded HTML file and extract all links
4. Insert each extracted link into the queue
5. Goto step 2, or stop once you reach some specified limit*/

	bool really = false;
	while (really == false) {
		string someMore;
		while (someMore != "no") {
			cout << "Enter base URL ";
			cin >> youAreEl;//getline(cin, youAreEl) works too

			hotmal[queuesize] = youAreEl;
			queuesize++;

			cout << "Input another URL? If not, type \"no\". ";
			cin >> someMore;
		}
		if (hotmal[0].empty()) {
			cout << "Nice try, bucko.\n";
		}
		else { really = true; }
	}

	
	
	
	while (hotmal[limit].empty()) {
		//search link

		link = hotmal[0];//does this make a pointer? I sure hope it doesn't

		queuesize = queuesize + linksearch(link);

		//if (hotmal[limit].empty()) {
			int copy = 0;
			while (!hotmal[copy].empty()) {
				hotmal[copy] = hotmal[copy + 1];
				copy++;
			}
		//}
	}
}


