#pragma once

#include <winsock2.h>


bool initialise_winsock();
void closeandclean_winsock();
SOCKET OpenTCPServerSocket(char* host, u_short port);
bool OpenUDPSocket(char* ip, u_short port);
bool SendUDP(char* buffer, int len);

int StartListenThreadOnTCPSocket(const char* host, unsigned int port, LPTHREAD_START_ROUTINE ThreadFunction);


