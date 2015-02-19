//
//  Socket.cpp
//
//  Created by Aaron Hartman on 17 SEP 14.
//  Copyright (c) 2014. All rights reserved.
//

#include "Socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sstream>
#include <stdexcept>
#include <unistd.h>

using std::cout;
using std::cerr;
using std::endl;
using std::stringstream;
using std::runtime_error;


///
/// Function name: server Socket constructor
/// Description: initializes a server socket to listenForCon for connections
/// Parameters: n/a
/// Returns: server Socket object
///
Socket::Socket()
{
    /**
    * Initialize the defaults for a server connection
    */
    _boundSocketDesc = NOT_CONNECTED;
    _currConnSocketDesc = NOT_CONNECTED;
    _ipAddress = "";
    _portNum = "0";
    _clientIpAddress = "";
    _clientPortNum = "";
    _data = NULL;
    _packetData = NULL;


    // IF setting up the server fails
    if(!setupServer())
        throw runtime_error("Failure setting up server!");

}

///
/// Function name: client Socket constructor
/// Description: initializes a server socket to listenForCon for connections
/// Parameters: n/a
/// Returns: server Socket object
///
Socket::Socket(string ipAddr, string port)
{
    /**
    * Initialize the defaults for a server connection
    */
    _boundSocketDesc = NOT_CONNECTED;
    _currConnSocketDesc = NOT_CONNECTED;
    _ipAddress = "";
    _portNum = "";
    _clientIpAddress = ipAddr;
    _clientPortNum = port;
    _data = NULL;
    _packetData = NULL;


    // IF setting up the server fails
    if(!setupClient())
        throw runtime_error("Failure setting up client!");

}

///
/// Function name: Destructor
/// Description: Reclaims allocated memory from the Socket data and packet data pointers,
///              where applicable.
///
Socket::~Socket()
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

    close(_boundSocketDesc); 
    close(_currConnSocketDesc);
}

///
/// Function name: initData
/// Description: Helper function to initialize the _data member to the specified max data
///              size. If it has already been initialized it will be deleted beforehand.
/// Parameters: n/a
/// Returns: void
///
void Socket::initData()
{
    // IF we have already allocated space for a read before
    if(_data != NULL)
        delete _data;

    _data = new char[MAX_DATA_SIZE];
}

///
/// Function name: initPacket
/// Description: Helper function to initialize the _packetData member to the specified Packet struct. 
/// If it has already been initialized it will be deleted beforehand.
/// Parameters: n/a
/// Returns: void
///
void Socket::initPacket()
{
    // IF we have already allocated space for a read before
    if(_packetData != NULL)
        delete _packetData;

    _packetData = new Packet;
}

///
/// Function name: setupServer
/// Description: initalizes a server socket to listenForCon for connections
/// Parameters: n/a
/// Returns: bool denoting its success or failure
///
bool Socket::setupServer()
{

    struct addrinfo hints;
    struct addrinfo *servinfo, *possible;
    int yes=1;


    // Clear out the hints struct
    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_UNSPEC;		// don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;	// TCP
    hints.ai_flags = AI_PASSIVE; 		// Use my IP since we are returning a server socket

    
    string port = "55535";

    // IF we fail to get the address info
    if ((getaddrinfo(NULL, port.c_str(), &hints, &servinfo)) != 0)
        throw runtime_error("Failure getting address info (server socket)");



    // loop through all the results and bind to the first we can
    for(possible = servinfo; possible != NULL; possible = possible->ai_next)
    {

        // IF there is an error establishing a socket to current result
        if ((_boundSocketDesc = socket(possible->ai_family, possible->ai_socktype,
                possible->ai_protocol)) == -1) {
            perror("Error establishing socket.");
            continue;
        }

        // IF setting the socket options failed
        if (setsockopt(_boundSocketDesc, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1)
        {
            perror("Error setting socket options.");
            exit(1);
        }

        // IF we couldn't bind to the socket
        if (bind(_boundSocketDesc, possible->ai_addr, possible->ai_addrlen) == -1) {
            close(_boundSocketDesc);
            perror("Error binding to socket. Socket closed.");
            continue;
        }

        break;
    }

    // IF every attempt to bind failed
    if (possible == NULL)
    {
        cerr << "Socket(class) failed to bind to the server socket." << endl;
        return 2;
    }
    // ELSE we did bind to a socket
    else
    {

        /**
        * Store the servers IP address
        */

        char actualAddress[INET6_ADDRSTRLEN];

        gethostname(actualAddress, sizeof actualAddress);


        struct hostent* hostAddrInfo = gethostbyname(actualAddress);


        if(hostAddrInfo == NULL)
        {
            cout << actualAddress << " is unavailable" << endl;
            return false;
        }

        in_addr * address = (in_addr * )hostAddrInfo->h_addr;


        // set this Socket objects ip address accordingly
        _ipAddress = inet_ntoa(* address);

        

        /**
         * Store the port the server is listening on
         */

        struct sockaddr_in sin;
        socklen_t addrlen = sizeof(sin);

        if(getsockname(_boundSocketDesc, (struct sockaddr *)&sin, &addrlen) == 0 &&
                sin.sin_family == AF_INET &&
                addrlen == sizeof(sin))
        {
            stringstream ss;
            ss << ntohs(sin.sin_port);
            ss >> _portNum;
        }

    }


    freeaddrinfo(servinfo); // all done with this structure


    return true;    // successfully setup server

}

///
/// Function name: setupClient
/// Description: initalizes a server socket to listenForCon for connections
/// Parameters: n/a
/// Returns: bool denoting its success or failure
///
bool Socket::setupClient()
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
        if ((_currConnSocketDesc = socket(possible->ai_family, possible->ai_socktype,
                possible->ai_protocol)) == -1)
        {

            perror("setupClient() socket call failed.");

            continue;

        }

        // IF connecting to the server fails
        if (connect(_currConnSocketDesc, possible->ai_addr, possible->ai_addrlen) == -1)
        {

            close(_currConnSocketDesc);

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


    return true;    // successfully made connection

}

///
/// Function name: listenForCon
/// Description: listens on the port this Socket is bound to for a new connection.
/// Parameters: n/a
/// Returns: bool denoting its success or failure
///
bool Socket::listenForCon()
{
    // verify that the socket is set
    checkBoundSocket();


    // IF our attempt to listenForCon fails
    if (listen(_boundSocketDesc, MAX_SOCKET_CONNECTIONS) == -1)
    {
        perror("Error Listening.");
        return false;
    }

    return true;
}

///
/// Function name: accept
/// Description: Attempts to make a connection until one is made.
/// Parameters: n/a
/// Returns: bool denoting its success. Note that until a connection is made
///			 this method will not exit.
///
bool Socket::acceptCon()
{

    // verify the socket is set
    checkBoundSocket();

    // IF the socket we assign connections to is open
    if(_currConnSocketDesc != NOT_CONNECTED)
    {
        close(_currConnSocketDesc);
        _currConnSocketDesc = NOT_CONNECTED;
    }


    socklen_t sin_size;
    struct sockaddr_storage their_addr;
    char clientAddress[INET6_ADDRSTRLEN];

    // WHILE we have NOT made a valid connection
    while(true)
    {
        // Accept an incoming connection
        sin_size = sizeof their_addr;
        _currConnSocketDesc = accept(_boundSocketDesc, (struct sockaddr *) &their_addr, &sin_size);


        // IF there was a failure accepting
        if (_currConnSocketDesc == -1)
        {
            perror("Error accepting connection.");   // print error
        }
        else
        {
            // Convert the connecting clients address to presentation form
            inet_ntop(their_addr.ss_family,
                    get_in_addr((struct sockaddr *)&their_addr),
                    clientAddress, sizeof clientAddress);


            // set this objects client IP address accordingly
            _clientIpAddress = clientAddress;

        /**
         * Store the port the server is listening on
         */

        struct sockaddr_in sin;
        socklen_t addrlen = sizeof(sin);

        if(getsockname(_currConnSocketDesc, (struct sockaddr *)&sin, &addrlen) == 0 &&
                sin.sin_family == AF_INET &&
                addrlen == sizeof(sin))
        {
            stringstream ss;
            ss << ntohs(sin.sin_port);
            ss >> _clientPortNum;
        }

            break;		// exit while
        }

    } // END WHILE

    return true;
}


///
/// Function name: read
/// Description: Attempts to read from the connected socket
/// Parameters: n/a
/// Returns: Address to a char array containing data obtained from the read.
///			 Returns NULL if irrecoverable error occurred.
///			 Note: char array is managed by destructor.
///
char& Socket::read()
{
    // verify the socket is set
    checkConnSocket();


    // WHILE we haven't received everything
    while(true)
    {
        /*	number of bytes remaining to be received	*/
        ssize_t numbytes;
        
        initData();
        
        // IF receiving a message failed
        if ((numbytes = recv(_currConnSocketDesc, _data, MAX_DATA_SIZE - 1, 0)) == -1)
            perror("Error receiving in read().");
        else
        {
            _data[numbytes] = '\0';
            break;
        }
    }


    return *_data;
}

///
/// Function name: readPacket
/// Description: Attempts to read from the connected socket
/// Parameters: n/a
/// Returns: Address to a packet struct containing data obtained from the read.
///          Packet struct is defined in Socket.h.
///          Returns NULL if irrecoverable error occurred.
///          Note: char array is managed by destructor.
///
Packet& Socket::readPacket()
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
        if ((numbytes = recv(_currConnSocketDesc, (void *)_packetData, sizeof(struct Packet), 0)) == -1)
            perror("Error receiving in read().");
        else
        {
            _packetData->message[_packetData->length] = '\0';

            break;
        }

    }


    return *_packetData;
}


///
/// Function name: write
/// Description: Attempts to write all data pointed to by the data pointer.
/// Parameters: Address to a char array containing data to be written to the Socket.
/// Returns: bool denoting its success or failure
///
bool Socket::write(char* dataToSend)
{
    // verify socket is connected
    checkConnSocket();



    // IF sending the message fails
    if (send(_currConnSocketDesc, &dataToSend, strlen(dataToSend), 0) == -1)
    {
        perror("Error writing data in send().");
        return false;
    }

    return true;

}

///
/// Function name: writePacket
/// Description: Attempts to write all data pointed to by the data pointer.
/// Parameters: Address to a Packet struct containing data to be written to the Socket.
/// Returns: bool denoting its success or failure
///
bool Socket::writePacket(Packet* dataToSend)
{

    // verify socket is connected
    checkConnSocket();


    // IF sending the message fails
    if (send(_currConnSocketDesc, (void *)dataToSend, sizeof(struct Packet), 0) == -1)
    {
        perror("Error writing data in send().");
        return false;
    }

    return true;

}

///
/// Function name: checkBoundSocket
/// Description: Verifies that the bound socket is indeed bound.
/// Parameters: n/a
/// Returns: bool denoting its success or failure
///
void Socket::checkBoundSocket()
{
    // IF our bound socket is not set
    if(_boundSocketDesc == NOT_CONNECTED)
        throw runtime_error("Error with 'bound' socket. Socket is not bound!");
}

///
/// Function name: checkConnSocket
/// Description: Verifies that the connected socket is indeed connected.
/// Parameters: n/a
/// Returns: bool denoting its success or failure
///
void Socket::checkConnSocket()
{
    // IF our bound socket is not set
    if(_currConnSocketDesc == NOT_CONNECTED)
        throw runtime_error("Error with socket. _currConnSocketDesc is not connected!");

}

///
/// Function to return the address in a sockaddr structure,
/// regardless of IPv4 or IPv6.
///
void* Socket::get_in_addr(struct sockaddr *sa)
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
void* Socket::get_in_port(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_port);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_port);
}
