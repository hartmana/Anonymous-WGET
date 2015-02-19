#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#define MAX_SOCKET_CONNECTIONS 5
#define MAX_DATA_SIZE 145
#define NOT_CONNECTED -1000

#include "PacketStruct.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>

using std::string;



///
/// Class to abstract the details away from using sockets.
///
class TCPSocket {

private:

    int _socketDescriptor;
    string _ipAddress;
    string _portNum;

    string _clientIpAddress;
    string _clientPortNum;

    unsigned char *_data;  
    PacketStruct *_packetData;  

    bool setupClient();
    void checkConnSocket();
    void initData(uint16_t size);
    void initPacket();
    void setClientInfo(struct sockaddr_storage& their_addr);
    void setHostInfo();
    void* get_in_addr(struct sockaddr *sa);
    void* get_in_port(struct sockaddr *sa);

public:

    // Destructor
    ~TCPSocket();

    // Constructor to wrap a socket file descriptor with a TCPSocket object
    TCPSocket(int tcpSocketFD, struct sockaddr_storage& their_addr);

    // Constructor to setup a client socket to a given port and IP
    TCPSocket(string ipAddr, string port);

    // Function to read from the connection and return the resulting char arrays address
    unsigned char* read(unsigned int& bytesRead);

    // Function to read from the connection and return the resulting Packet structs address
    PacketStruct* readPacket();

    // Function to write to the connection
    bool write(unsigned char* dataToSend, size_t dataToSendLength);

    // Function to write to the connection
    bool writePacket(PacketStruct *data);


    inline int getSocketDescriptor() { return _socketDescriptor; }

    inline string getHostIpAddress() { return _ipAddress; }

    inline string getClientIpAddress() { return _clientIpAddress; }

    inline string getHostPort() { return _portNum; }

    inline string getClientPort() { return _clientPortNum; }

    inline unsigned char* getData() { return _data; }

    inline PacketStruct* getPacket() { return _packetData; }


};

#endif
