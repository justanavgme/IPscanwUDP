#include "IPscanwUDP.h"

static string ip, submask;

void getmyIP()
{
	string line;
	ifstream IPFile;
	int offset;
	const char* search0 = "IPv4 Address. . . . . . . . . . . :";      // search pattern
	const char* search1 = "Subnet Mask . . . . . . . . . . . :";

	system("ipconfig > ip.txt");

	IPFile.open("ip.txt");
	if (IPFile.is_open())
	{
		while (!IPFile.eof())
		{
			getline(IPFile, line);
			if ((offset = line.find(search0, 0)) != string::npos)
			{
				//   IPv4 Address. . . . . . . . . . . : 1
				//1234567890123456789012345678901234567890     
				line.erase(0, 39);
				ip = line;
				//cout << line << endl;
			}
			else if ((offset = line.find(search1, 0)) != string::npos)
			{
				//   Subnet Mask . . . . . . . . . . . : 2
				//1234567890123456789012345678901234567890     
				line.erase(0, 39);
				submask = line;
				//cout << line << endl;
			}
		}
		IPFile.close();
	}
}

string getSubnet(string ip, string submask)
{
	char buf[INET_ADDRSTRLEN];

	// Store ip and submask in ULONG type for calculation
	struct sockaddr_in ipSocket;
	inet_pton(AF_INET, ip.c_str(), &(ipSocket.sin_addr.S_un.S_addr));
	struct sockaddr_in submaskSocket;
	inet_pton(AF_INET, submask.c_str(), &(submaskSocket.sin_addr.S_un.S_addr));
	struct sockaddr_in subnetSocket;
	subnetSocket.sin_addr.S_un.S_addr = (ipSocket.sin_addr.S_un.S_addr & submaskSocket.sin_addr.S_un.S_addr) | (~submaskSocket.sin_addr.S_un.S_addr);

	// Convert & store subnet addr in buf[] Then prints it.
	inet_ntop(AF_INET, &(subnetSocket.sin_addr), buf, INET_ADDRSTRLEN);

	return buf;

}

int w_recvfrom(unsigned s, void* ph, int len, int flags, sockaddr* server, int* serverLength)
{
	fd_set r, e;
	struct timeval tval;
	int i, retval, done = 0, total = 0;

	while (!done) {
		FD_ZERO(&r); FD_ZERO(&e);
		FD_SET(s, &r); FD_SET(s, &e);
		tval.tv_sec = 0;
		tval.tv_usec = 0;
		retval = select(s + 1, &r, NULL, &e, &tval);
		if (retval == 0) {
			//Timeout elapsed with no activity
			return -1;
		}
		if (retval == -1) {
			//Socket error with select().
			return -1;
		}
		if (FD_ISSET(s, &e)) {
			//some other TCP/IP error triggered select()
			return -1;
		}
		if (FD_ISSET(s, &r)) {
			//data is available on the socket  Gather it up
			//i = recv(s, ((char*)ph) + total, len - total, flags);
			i = recvfrom(s, ((char*)ph) + total, len - total, 0, (sockaddr*) & (*server), &(*serverLength));
			if (i == -1) {
				//Socket error with recv().
				return -1;
			}
			if (i == 0) {
				//something is wrong.  select trigged but there is no data
				return -1;
			}
			//loop until all requested data is received
			total += i;
			//if (total == len) {
			done = 1;
			//}
		}
	}//done
	return total;
}

int getServerIP(string* serverIP, int TIMEOUT, unsigned short int PORT)
{
	// Get subnet address for broadcasting
	getmyIP();
	string my_subnet = getSubnet(ip, submask);

	// Startup Winsock
	WSADATA data;
	WORD version = MAKEWORD(2, 2);
	int ws0k = WSAStartup(version, &data);
	if (ws0k != 0)
	{
		//cerr << "Can't start winsock! Quitting." << endl;
		return -1;
	}

	// Create socket
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);

	// setsockopt SO_BROADCAST
	char broadcast = '1';
	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0)
	{
		//cerr << "Error in setting SO_BROADCAST option! Quitting." << endl;
		closesocket(sock);
		WSACleanup();
		return -1;
	}

	// sockaddr_in for sender & receiver
	struct sockaddr_in Recv_addr;
	struct sockaddr_in Sender_addr;
	int len = sizeof(struct sockaddr_in);
	Recv_addr.sin_family = AF_INET;
	Recv_addr.sin_port = htons(PORT);
	inet_pton(AF_INET, my_subnet.c_str(), &Recv_addr.sin_addr);

	// Message and buffer definitions
	char sendMSG[] = "Broadcast message from Client";
	char recvbuff[50] = "";
	int recvbufflen = 50;

	auto start = high_resolution_clock::now();
	auto finish = high_resolution_clock::now();
	duration<double> elapsed = finish - start;
	start = high_resolution_clock::now();
	while (true)
	{
		// sendto()
		sendto(sock, sendMSG, strlen(sendMSG) + 1, 0, (sockaddr*)& Recv_addr, sizeof(Recv_addr));
		// recvfrom()
		int recvOk = w_recvfrom(sock, recvbuff, recvbufflen, 0, (sockaddr*)& Recv_addr, &len);
		if (recvOk != -1)
		{
			//cout << "Received from Server successfully!" << endl;
			sendto(sock, sendMSG, strlen(sendMSG) + 1, 0, (sockaddr*)& Recv_addr, sizeof(Recv_addr));
			// Display message received from server and server's info
			char ServerIP[256];
			ZeroMemory(ServerIP, 256);
			inet_ntop(AF_INET, &Recv_addr.sin_addr, ServerIP, 256);
			//cout << "Message received from " << ServerIP << ": " << recvbuff << endl;
			*serverIP = ServerIP;
			closesocket(sock);
			WSACleanup();
			return 1;
		}
		finish = high_resolution_clock::now();
		elapsed = finish - start;
		if (elapsed.count() > TIMEOUT)
		{
			//cout << "Receiving TIMEOUT!" << endl;
			closesocket(sock);
			WSACleanup();
			return 0;
		}
	}
}

int getClientIP(string* clientIP, int TIMEOUT, unsigned short int PORT)
{
	// Startup Winsock
	WSADATA data;
	WORD version = MAKEWORD(2, 2);
	int ws0k = WSAStartup(version, &data);
	if (ws0k != 0)
	{
		//cerr << "Can't start winsock! Quitting." << endl;
		return -1;
	}

	// Create socket
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);

	// setsockopt SO_BROADCAST
	char broadcast = '1';
	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0)
	{
		//cerr << "Error in setting SO_BROADCAST option! Quitting." << endl;
		closesocket(sock);
		WSACleanup();
		return -1;
	}
	// sockaddr_in for receiver & sender
	struct sockaddr_in Recv_addr;
	struct sockaddr_in Sender_addr;
	int len = sizeof(struct sockaddr_in);
	Recv_addr.sin_family = AF_INET;
	Recv_addr.sin_port = htons(PORT);
	Recv_addr.sin_addr.s_addr = INADDR_ANY;

	// Message and buffer definitions
	char recvbuff[50];
	int recvbufflen = 50;
	char sendMSG[] = "Broadcast message from Server";

	// Bind socket
	if (bind(sock, (sockaddr*)& Recv_addr, sizeof(Recv_addr)) < 0)
	{
		//cerr << "Error in Binding! Quitting. Error code: " << WSAGetLastError() << endl;
		closesocket(sock);
		WSACleanup();
		return -1;
	}

	auto start = high_resolution_clock::now();
	auto finish = high_resolution_clock::now();
	duration<double> elapsed = finish - start;
	start = high_resolution_clock::now();
	while (true)
	{
		// sendto()
		sendto(sock, sendMSG, strlen(sendMSG) + 1, 0, (sockaddr*)& Sender_addr, sizeof(Sender_addr));
		// recvfrom()
		int recvOk = w_recvfrom(sock, recvbuff, recvbufflen, 0, (sockaddr*)& Sender_addr, &len);
		if (recvOk != -1)
		{
			//cout << "Received from Client successfully!" << endl;
			sendto(sock, sendMSG, strlen(sendMSG) + 1, 0, (sockaddr*)& Sender_addr, sizeof(Sender_addr));
			char ClientIP[256];
			ZeroMemory(ClientIP, 256);
			inet_ntop(AF_INET, &Sender_addr.sin_addr, ClientIP, 256);
			//cout << "Message received from " << ClientIP << ": " << recvbuff << endl;
			*clientIP = ClientIP;
			closesocket(sock);
			WSACleanup();
			return 1;
		}
		finish = high_resolution_clock::now();
		elapsed = finish - start;
		if (elapsed.count() > TIMEOUT)
		{
			//cout << "Receiving TIMEOUT!" << endl;
			closesocket(sock);
			WSACleanup();
			return 0;
		}
	}
}