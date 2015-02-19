#ifndef SOCKET_H
#define SOCKET_H

#define MAX_SOCKET_CONNECTIONS 5
#define MAX_DATA_SIZE 145
#define MESSAGE_LENGTH 140
#define NOT_CONNECTED -1000

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

using std::string;


struct Packet {
    uint16_t version;
    uint16_t length;
    char message[MESSAGE_LENGTH];
};


///
/// Class to abstract the details away from using sockets.
///
class Socket {

private:


    int _boundSocketDesc;
    int _currConnSocketDesc;
    string _ipAddress;
    string _portNum;

    string _clientIpAddress;
    string _clientPortNum;

    char *_data;            // This will be filled as needed by the read function
    Packet *_packetData;    // This will be filled as needed by the readPacket function


    bool setupServer();
    bool setupClient();
    void checkBoundSocket();
    void checkConnSocket();
    void initData();
    void initPacket();
    void* get_in_addr(struct sockaddr *sa);
    void* get_in_port(struct sockaddr *sa);

public:

    // Destructor
    ~Socket();

    // Default constructor initializes a server socket
    Socket();

    // Constructor to setup a client socket to a given port and IP
    Socket(string ipAddr, string port);

    // Function to read from the connection and return the resulting char arrays address
    char& read();

    // Function to read from the connection and return the resulting Packet structs address
    Packet& readPacket();

    // Function to listenForCon for an incoming connection
    bool listenForCon();

    // Function to accept a connection on the socket we are listening to
    bool acceptCon();

    // Function to write to the connection
    bool write(char *data);

    // Function to write to the connection
    bool writePacket(Packet *data);


    inline int getBoundSocketDescriptor() { return _boundSocketDesc; }

    inline int getConnSocketDescriptor() { return _currConnSocketDesc; }

    inline string getHostIpAddress() { return _ipAddress; }

    inline string getClientIpAddress() { return _clientIpAddress; }

    inline string getPort() { return _portNum; }

    inline string getClientPort() { return _clientPortNum; }

    inline char* getData() { return _data; }

    inline Packet* getPacket() { return _packetData; }


};

#endif
