#ifndef TCPSERVERSOCKET_H
#define TCPSERVERSOCKET_H

#define MAX_SOCKET_CONNECTIONS 5
#define NOT_CONNECTED -1000

#include "TCPSocket.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>



using std::string;



///
/// Class to abstract the details away from using sockets.
///
class TCPServerSocket {

private:

    int _boundSocketDesc;
    int _currConnSocketDesc;
    string _ipAddress;
    string _portNum;

    string _clientIpAddress;
    string _clientPortNum;

    bool setupServer();
    void checkBoundSocket();
    void checkConnSocket();
    void* get_in_addr(struct sockaddr *sa);
    void* get_in_port(struct sockaddr *sa);

public:

    // Destructor
    ~TCPServerSocket();

    // Default constructor initializes a server socket
    TCPServerSocket(string port="0");

    // Function to listenForCon for an incoming connection
    bool listenForCon();

    // Function to accept a connection on the socket we are listening to
    TCPSocket* acceptCon();


    inline int getBoundSocketDescriptor() { return _boundSocketDesc; }

    inline int getConnSocketDescriptor() { return _currConnSocketDesc; }

    inline string getHostIpAddress() { return _ipAddress; }

    inline string getClientIpAddress() { return _clientIpAddress; }

    inline string getPort() { return _portNum; }

    inline string getClientPort() { return _clientPortNum; }


};

#endif
