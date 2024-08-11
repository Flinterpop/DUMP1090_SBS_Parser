

#pragma comment (lib, "Ws2_32.lib")

#include <winsock2.h>
#include "Ws2tcpip.h"  //to use InetPton

#include <iostream>  //for printf


wchar_t* convertCharArrayToLPCWSTR(const char* charArray);

static sockaddr_in g_udp_destination;
static int g_udp_socket;
SOCKET TCPClientSocket;

bool initialise_winsock()
{
    WSADATA ws;
    printf("Initialising Winsock...\r\n");
    if (WSAStartup(MAKEWORD(2, 2), &ws) != 0)
    {
        printf("Failed. Error Code: %d\r\n", WSAGetLastError());
        return true;
    }
    printf("         Winsock Initialized...\r\n");
    return false;
}


void closeandclean_winsock()
{
    WSACleanup();
}


SOCKET OpenTCPServerSocket(char* host, u_short port)
{
    // Create client socket 
    SOCKET TCPClientSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (TCPClientSocket == INVALID_SOCKET) {
        printf("Error, I cannot create TCP socket\r\n");
        return EXIT_FAILURE;
    }

    // setup address structure
    SOCKADDR_IN  addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;

    //#pragma warning(disable:4996) 
    //addr.sin_addr.s_addr = inet_addr(host);
    //InetPton(AF_INET, __TEXT("192.168.1.133"), &addr.sin_addr.s_addr);
    PCWSTR p = convertCharArrayToLPCWSTR(host);
    InetPton(AF_INET, p, &addr.sin_addr.s_addr);
    addr.sin_port = htons(port);

    // Attemp to connect to server and exit on failure. 
    int connval = connect(TCPClientSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if (connval == SOCKET_ERROR) {
        printf("Error: cannot connect to server.\r\n");
        //Returns status code other than zero
        return EXIT_FAILURE;
    }
    return TCPClientSocket;
}



static wchar_t* convertCharArrayToLPCWSTR(const char* charArray)
{
    wchar_t* wString = new wchar_t[4096];
    MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
    return wString;
}


bool OpenUDPSocket(char* ip, u_short port)//works for UDP, broadcast and multicast as tested
{
    // create socket
    if ((g_udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR) // <<< UDP socket
    {
        printf("socket() failed with error code: %d", WSAGetLastError());
        return true;
    }

    // setup address structure
    memset((char*)&g_udp_destination, 0, sizeof(g_udp_destination));
    g_udp_destination.sin_family = AF_INET;
    g_udp_destination.sin_port = htons(port);

    //old way (deprecated)
    //#pragma warning(disable:4996) 
    //m_server.sin_addr.S_un.S_addr = inet_addr(ip);

    //new way - convert char * to PCWSTR if ip is a variable, use __TEXT macro is IP is a literal text string
    PCWSTR p = convertCharArrayToLPCWSTR(ip);
    InetPton(AF_INET, p, &g_udp_destination.sin_addr.s_addr);
    //InetPton(AF_INET, __TEXT("192.168.1.133"), &g_udp_destination.sin_addr.s_addr);
    return false;
}

bool SendUDP(char* buffer, int len)
{
    if (sendto(g_udp_socket, (const char*)buffer, len, 0, (sockaddr*)&g_udp_destination, sizeof(sockaddr_in)) == SOCKET_ERROR)
    {
        printf("sendto() failed with error code: %d\r\n", WSAGetLastError());
        return true;
    }
    //printf("Sent message of length %d\r\n", len);
    return false;
}


int StartListenThreadOnTCPSocket(const char* host, unsigned int port, LPTHREAD_START_ROUTINE ThreadFunction)
{
    //assumes initialise_winsock(); has beenb called earlier
    TCPClientSocket = OpenTCPServerSocket((char*)host, port);
    if (EXIT_FAILURE == TCPClientSocket) return EXIT_FAILURE;

    //HANDLE hThread = CreateThread(NULL, 0, TCPListenThread, (LPVOID)&TCPClientSocket, 0, NULL);
    HANDLE hThread = CreateThread(NULL, 0, ThreadFunction, (LPVOID)&TCPClientSocket, 0, NULL);

    if (hThread == NULL)
    {
        printf("Thread not created\r\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


