#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <WS2tcpip.h>
#include <chrono>

#pragma comment (lib, "ws2_32.lib")
using namespace std;
using namespace chrono;

extern void getmyIP();
extern string getSubnet(string ip, string submask);
extern int w_recvfrom(unsigned s, void* ph, int len, int flags, sockaddr* server, int* serverLength);
extern int getServerIP(string* serverIP, int TIMEOUT, unsigned short int PORT);
extern int getClientIP(string* clientIP, int TIMEOUT, unsigned short int PORT);