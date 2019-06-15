# IPscanwUDP
A C++ library for UDP discovery.
## General description
A Library for IP scanning between ONE Server and ONE Client in Local Network using UDP (UDP discovery method).
(Utilizes Winsock 2 - `WS2tcpip.h`)
Can be used to:
- Scan Server & Client IP.
- Scan and return host-computer's IP address & subnet mask.
- Calculate subnet broadcasting IP.
- Non-blocking `recvfrom()` operation with `w_recvfrom()`.
## Detailed description
### Scan Server & Client IP
1. Run both programs simultaneously on two computers connected to a same network (can be Wifi or Ethernet).
2. Doesn't matter if Client or Server runs first.
3. If no broadcasting datagram received from one another > Timeout.
- Usage: Function `getServerIP`, `getClientIP` (adapted from Dr. Darabant's [example](http://www.cs.ubbcluj.ro/~dadi/compnet/labs/lab3/udp-broadcast.html) for his Computer Networks class)
  - Argument 1: `string* serverIP`        > string to store Server's IP when scan successfully.
  - Argument 2: `int TIMEOUT`             > TIMEOUT duration in seconds
  - Argument 3: `unsigned short int PORT` > PORT number to use (must match between server & client side, value must be from 0-65535)
  - Same arguments description goes for `getClientIP`.
- Returns:
  - `-1` if error.
  - ` 0` if timeout.
  - ` 1` if success
- Example codes:
  - Client's side example:
```
#include <iostream>
#include "IPscanwUDP.h"

using namespace std;

int main()
{
	string myServerIP;
	// Output string = myClientIP, timeout = 5s, port used = 9009
	int getOK = getServerIP(&myServerIP, 5, 9009);
	if (getOK == -1)
		cout << "Receiving ERROR!" << endl;
	else if (getOK == 0)
		cout << "Receiving TIMEOUT..." << endl;
	else //getOK = 1
	{
		cout << "Receiving SUCCESS! Server's IP = " << myServerIP << endl;
	}
	cout << "Press any key to CLOSE program..." << endl;
	cin.get();	// Stop the program to see output

	return 0;
}
```
  - Server's side example:
```
#include <iostream>
#include "IPscanwUDP.h"

using namespace std;

int main()
{
	string myClientIP;
	// Output string = myClientIP, timeout = 5s, port used = 9009
	int getOK = getClientIP(&myClientIP, 5, 9009);
	if (getOK == -1)
		cout << "Receiving ERROR!" << endl;
	else if (getOK == 0)
		cout << "Receiving TIMEOUT..." << endl;
	else //getOK = 1
	{
		cout << "Receiving SUCCESS! Client's IP = " << myClientIP << endl;
	}
	cout << "Press any key to CLOSE program..." << endl;
	cin.get();	// Stop the program to see output

	return 0;
}
```
### Scan and return host-computer's IP address & subnet mask.
**Security risk:** This function method is to use `fstream`, creates a `ip.txt` file to store `ipconfig` results. Then find IPv4 address and subnet mask in this file.
- Usage: Function `getmyIP()` (Adapted from SamuelAdams's answer at [cplusplus forum](http://www.cplusplus.com/forum/windows/82534/))
  - Argument 1: `string *ip` > string to store computer's IPv4 address.
  - Argument 2: `string *submask` > string to store computer's Subnet mask address.
- Returns: Nothing.
- Example code:
```
#include <iostream>
#include "IPscanwUDP.h"

using namespace std;

int main()
{
	string ip, submask;
	getmyIP(&ip, &submask);
	cout << "My computer's IP: " << ip << endl;
	cout << "My computer's subnet mask: " << submask << endl;
	return 0;
}
```
### Calculate subnet broadcasting IP.
- Usage: Function `getSubnet()`
  - Argument 1: `string ip` > computer's IP for calculation.
  - Argument 1: `string submask` > computer's Subnet mask for calculation.
- Returns: a `string`-typed value that contains the Subnet broadcasting IP
- Example code:
```
#include <iostream>
#include "IPscanwUDP.h"

using namespace std;

int main()
{
	string ip, submask;
	getmyIP(&ip, &submask);
	string my_subnet = getSubnet(ip, submask);
	cout << "Subnet broadcasting address = " << my_subnet << endl;
	return 0;
}
```
### Non-blocking `recvfrom()` operation with `w_recvfrom()`.
- Usage: Function `w_recvfrom()` (Adapted from hoxsiew's answer at [codeguru forum](http://forums.codeguru.com/showthread.php?476227-recv()-hang))
  - Arguments: Exactly the same as Winsock's `recvfrom()` function (See Microsoft's [documentation](https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-recvfrom))
  - **Important FEATURE** Default is NO timeout. If neccessary, timeout can be set by editing the source code in `IPscanwUDP.cpp`. Change `tval.tv_sec` and `tval.tv_usec` values to set your desired timeout.
- Returns:
  - `-1` if timeout or error
  - An `int`-typed value representing the total number of received characters in the message.
  - **Important NOTICE:** `w_recvfrom()` does not output 0 when timeout as in the original adapted source. However, function's output can be easily changed to your desired value by modifying the source code as mentioned above.
- Example code: See Microsoft's [documentation](https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-recvfrom)
In a nutshell: Replace `recvfrom()` by `w_recvfrom()` whenever the non-blocking operation is desired.
## Disclaimer
This library is orginally written for used in a Chess game via LAN (school side-project). There is no license and NO security measure in this library. Please use it as you see fit!
