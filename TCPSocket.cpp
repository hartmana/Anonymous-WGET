//
//  Socket.cpp
//
//  Created by Aaron Hartman on 17 SEP 14.
//  Copyright (c) 2014. All rights reserved.
//

#include "TCPSocket.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sstream>
#include <stdexcept>
#include <unistd.h>


#define MAX_LENGTH 100

using std::cout;
using std::cerr;
using std::endl;
using std::stringstream;
using std::runtime_error;


///
/// Function name: socket constructor
/// Description: initializes a TCPSocket object around a TCP socket file descriptor.
/// Parameters: int denoting the file descriptor of the connection.
///             struct suckaddr_storage reference
/// Returns: TCPSocket object
///
TCPSocket::TCPSocket(int tcpSocketFD, struct sockaddr_storage& their_addr)
{
    /**
    * Initialize the defaults for a server connection
    */
    _socketDescriptor = tcpSocketFD;
    _ipAddress = "";
    _portNum = "0";
    _clientIpAddress = "";
    _clientPortNum = "";
    _data = NULL;
    _packetData = NULL;


    /**
     * Set the client and host IP and port number of this TCPSocket
     */
    setClientInfo(their_addr);
    setHostInfo();

}

///
/// Function name: client Socket constructor
/// Description: initializes a server socket to listen for connections
/// Parameters: n/a
/// Returns: TCPSocket client object
///
TCPSocket::TCPSocket(string ipAddr, string port)
{
    /**
    * Initialize the defaults for a server connection
    */
    _socketDescriptor = NOT_CONNECTED;
    _ipAddress = "";
    _portNum = "";
    _clientIpAddress = ipAddr;
    _clientPortNum = port;
    _data = NULL;
    _packetData = NULL;


    // IF setting up the client fails
    if(!setupClient())
        throw runtime_error("Failure setting up client!");

}

///
/// Function name: Destructor
/// Description: Reclaims allocated memory from the Socket data and packet data pointers,
///              where applicable.
///
TCPSocket::~TCPSocket()
{ 
    if(_data != NULL)
    {
        delete(_data); 
        _data = NULL;
    } 

    if(_packetData != NULL) 
    {
        delete(_packetData); 
        _packetData = NULL; 
    }

    shutdown(_socketDescriptor, SHUT_WR);
}

///
/// Function name: setClientInfo
/// Description: Helper function to set the IP address and Port number of the client computer this
///              socket is connected to.
/// Parameters: n/a
/// Returns: void
///
void TCPSocket::setClientInfo(struct sockaddr_storage& their_addr)
{
    /**
     * Verify the passed in file descriptor and get the client information
     */

    char clientAddress[INET6_ADDRSTRLEN];



    // IF there was a failure accepting
    if (_socketDescriptor == -1)
    {
        throw runtime_error("TCPSocket: Given an invalid socket descriptor.");
    }
    else 
    {
        // Convert the connecting clients address to presentation form
        inet_ntop(their_addr.ss_family,
                get_in_addr((struct sockaddr *) &their_addr),
                clientAddress, sizeof clientAddress);


        // set this objects client IP address accordingly
        _clientIpAddress = clientAddress;

        /**
        * Store the port the server is listening on
        */

        struct sockaddr_in sin;
        socklen_t addrlen = sizeof(sin);

        if (getsockname(_socketDescriptor, (struct sockaddr *) &sin, &addrlen) == 0 &&
                sin.sin_family == AF_INET &&
                addrlen == sizeof(sin)) {
            stringstream ss;
            ss << ntohs(sin.sin_port);
            ss >> _clientPortNum;
        }
    }
}

///
/// Function name: setHostInfo
/// Description: Helper function to set the IP address and Port number of the host computer
///              this client TCPSocket is running on.
/// Parameters: n/a
/// Returns: void
///
void TCPSocket::setHostInfo()
{
    char actualAddress[INET6_ADDRSTRLEN];

    gethostname(actualAddress, sizeof actualAddress);


    struct hostent* hostAddrInfo = gethostbyname(actualAddress);


    if(hostAddrInfo == NULL)
    {
        cout << actualAddress << " is unavailable" << endl;
    }

    in_addr * address = (in_addr * )hostAddrInfo->h_addr;


    // set this Socket objects ip address accordingly
    _ipAddress = inet_ntoa(* address);



    /**
     * Store the port this socket is at
     */

    struct sockaddr_in sin;
    socklen_t addrlen = sizeof(sin);

    if(getsockname(_socketDescriptor, (struct sockaddr *)&sin, &addrlen) == 0 &&
            sin.sin_family == AF_INET &&
            addrlen == sizeof(sin))
    {
        stringstream ss;
        ss << ntohs(sin.sin_port);
        ss >> _portNum;
    }
}

///
/// Function name: initData
/// Description: Helper function to initialize the _data member to the specified max data
///              size. If it has already been initialized it will be deleted beforehand.
/// Parameters: n/a
/// Returns: void
///
void TCPSocket::initData(uint16_t size)
{

    // IF we have already allocated space for a read before
    if(_data != NULL)
        delete _data;

    _data = new unsigned char[size];

}


///
/// Function name: initPacket
/// Description: Helper function to initialize the _packetData member to the specified Packet struct. 
/// If it has already been initialized it will be deleted beforehand.
/// Parameters: n/a
/// Returns: void
///
void TCPSocket::initPacket()
{
    // IF we have already allocated space for a read before
    if(_packetData != NULL)
        delete _packetData;

    _packetData = new PacketStruct;
}


///
/// Function name: setupClient
/// Description: initalizes a server socket to listenForCon for connections
/// Parameters: n/a
/// Returns: bool denoting its success or failure
///
bool TCPSocket::setupClient()
{

    struct addrinfo hints;
    struct addrinfo *servinfo, *possible;
    int rv;


    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // IF filling the addrinfo struct fails
    if ((rv = getaddrinfo(_clientIpAddress.c_str(), _clientPortNum.c_str(), &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));

        cout << "Address: " << _clientIpAddress << ":" << _clientPortNum << endl;
        return false;
    }

    // loop through all the results and connect to the first we can
    for(possible = servinfo; possible != NULL; possible = possible->ai_next)
    {
        // IF initializing the socket fails
        if ((_socketDescriptor = socket(possible->ai_family, possible->ai_socktype,
                possible->ai_protocol)) == -1)
        {

            perror("setupClient() socket call failed.");

            continue;

        }

        // IF connecting to the server fails
        if (connect(_socketDescriptor, possible->ai_addr, possible->ai_addrlen) == -1)
        {

            close(_socketDescriptor);

            perror("setupClient() connect call failed.");

            continue;

        }

        break;
    }
    // IF we didn't make any connection
    if (possible == NULL) {
        fprintf(stderr, "setupClient(): Failed to connect to %s.\n", _clientIpAddress.c_str());
        return false;
    }


    freeaddrinfo(servinfo); // all done with this structure

    // Set the host info for later use
    setHostInfo();

    return true;    // successfully made connection

}


///
/// Function name: read
/// Description: Attempts to read from the connected socket
/// Parameters: unsigned int& to signify to the caller the size of the data returned.
/// Returns: Address to a char array containing data obtained from the read.
///          Note: char array is managed by destructor.
///
unsigned char* TCPSocket::read(unsigned int& bytesRead)
{

    // verify the socket is set
    checkConnSocket();


    ssize_t totalBytesRecv = 0;
    ssize_t numBytes = 0;

    unsigned char incLength[2];     // length of incoming data

    // WHILE we haven't received everything
    while(totalBytesRecv < 2)
    {
        
        numBytes = recv(_socketDescriptor, static_cast<void *>(incLength + totalBytesRecv), (2 - totalBytesRecv), 0);

        // IF receiving a message failed
        if(numBytes == -1)
        {
            perror("Error receiving in read().");
        }
        else
        {
            totalBytesRecv += numBytes;
        }

    }

    // convert the length from char to int
    uint16_t messageLength = incLength[0] << 8;
    messageLength |= incLength[1];

    cout << "messageLength: " << messageLength << endl;


    initData(messageLength);
    
    totalBytesRecv = 0;

    // WHILE we haven't received everything
    while(totalBytesRecv < (messageLength - 1))
    {
        
        numBytes = recv(_socketDescriptor, static_cast<void *>(_data + totalBytesRecv), ((messageLength - 1) - totalBytesRecv), 0);
        
        // IF receiving a message failed
        if(numBytes == -1)
        {
            perror("Error receiving in read().");
        }
        else
        {
            totalBytesRecv += numBytes;
        }

    }

    _data[messageLength] = '\0';

    bytesRead = messageLength;

    return  _data;    

}



///
/// Function name: readPacket
/// Description: Attempts to read from the connected socket
/// Parameters: n/a
/// Returns: Address to a packet struct containing data obtained from the read.
///          Packet struct is defined in Socket.h.
///          Note: char array is managed by destructor.
///
PacketStruct* TCPSocket::readPacket()
{
    // verify the socket is set
    checkConnSocket();


    // WHILE we continue getting errors on receiving
    while(true)
    {
        /*  number of bytes remaining to be received    */
        ssize_t numbytes = 0;
        
        initPacket();
        
        // IF receiving a message failed
        if ((numbytes = recv(_socketDescriptor, static_cast<void *>(_packetData), sizeof(struct PacketStruct), 0)) == -1)
            perror("Error receiving in read().");
        else
        {
            _packetData->message[_packetData->length] = '\0';

            break;
        }

    }


    return _packetData;
}


///
/// Function name: write
/// Description: Attempts to write all data pointed to by the data pointer. Assumes first two bytes
///              are length and the end of the data to send is null terminated.
/// Parameters: Address to a char array containing data to be written to the Socket.
/// Returns: bool denoting its success or failure
///
bool TCPSocket::write(unsigned char* dataToSend, size_t dataToSendLength)
{
    // verify socket is connected
    checkConnSocket();


    /************************************************************************
     * Change this so that the length of the data is encoded into an array and 
     * sent first from within write(). A separate send will be used for the 
     * actual data.
     ************************************************************************/



    ssize_t bytesSent = 0;
    size_t totalBytesSent = 0;


    // while(totalBytesSent < messageLength)
    while(totalBytesSent < dataToSendLength)
    {
        
        
        bytesSent = send(_socketDescriptor, static_cast<void *>(dataToSend + totalBytesSent), (dataToSendLength - totalBytesSent), 0);
        
        
        // IF sending the message fails
        if(bytesSent == -1)
        {
            perror("Error writing data in write().");
            return false;
        }
        else
        {
            totalBytesSent += bytesSent;
        }
        
    }

    cout << "wrote " << totalBytesSent << " bytes" << endl;

    return true;

}




///
/// Function name: writePacket
/// Description: Attempts to write all data pointed to by the data pointer.
/// Parameters: Address to a Packet struct containing data to be written to the Socket.
/// Returns: bool denoting its success or failure
///
bool TCPSocket::writePacket(PacketStruct* dataToSend)
{

    // verify socket is connected
    checkConnSocket();


    // IF sending the message fails
    if (send(_socketDescriptor, static_cast<void *>(dataToSend), sizeof(struct PacketStruct), 0) == -1)
    {
        perror("Error writing data in send(). ");
        return false;
    }

    return true;

}



///
/// Function name: checkConnSocket
/// Description: Verifies that the connected socket is indeed connected.
/// Parameters: n/a
/// Returns: bool denoting its success or failure
///
void TCPSocket::checkConnSocket()
{
    // IF our bound socket is not set
    if(_socketDescriptor == NOT_CONNECTED)
        throw runtime_error("Error with socket. _currConnSocketDesc is not connected!");

}

///
/// Function to return the address in a sockaddr structure,
/// regardless of IPv4 or IPv6.
///
void*TCPSocket::get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

///
/// Function to return the port in a sockaddr structure,
/// regardless of IPv4 or IPv6.
///
void*TCPSocket::get_in_port(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_port);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_port);
}
