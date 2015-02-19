//
//  TCPServerSocket.cpp
//
//  Created by Aaron Hartman on 17 SEP 14.
//  Copyright (c) 2014. All rights reserved.
//

#include "TCPServerSocket.h"


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
/// Parameters: string containing the port the server socket should be established on. Defaults to 0.
/// Returns: server Socket object
///
TCPServerSocket::TCPServerSocket(string port)
{
    /**
    * Initialize the defaults for a server connection
    */
    _boundSocketDesc = NOT_CONNECTED;
    _currConnSocketDesc = NOT_CONNECTED;
    _ipAddress = "";
    _portNum = port;
    _clientIpAddress = "";
    _clientPortNum = "";


    // IF setting up the server fails
    if(!setupServer())
        throw runtime_error("Failure setting up server!");

}



///
/// Function name: Destructor
/// Description: Reclaims allocated memory from the Socket data and packet data pointers,
///              where applicable.
///
TCPServerSocket::~TCPServerSocket()
{
    close(_boundSocketDesc);
    close(_currConnSocketDesc);
}



///
/// Function name: setupServer
/// Description: initalizes a server socket to listenForCon for connections
/// Parameters: n/a
/// Returns: bool denoting its success or failure
///
bool TCPServerSocket::setupServer()
{

    struct addrinfo hints;
    struct addrinfo *servinfo, *possible;
    int yes=1;


    // Clear out the hints struct
    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_UNSPEC;		// don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;	// TCP
    hints.ai_flags = AI_PASSIVE; 		// Use my IP since we are returning a server socket


       
    // IF we fail to get the address info
    if ((getaddrinfo(NULL, _portNum.c_str(), &hints, &servinfo)) != 0)
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

    // start listening for connections
    listenForCon();


    return true;    // successfully setup server

}


///
/// Function name: listenForCon
/// Description: listens on the port this Socket is bound to for a new connection.
/// Parameters: n/a
/// Returns: bool denoting its success or failure
///
bool TCPServerSocket::listenForCon()
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
TCPSocket* TCPServerSocket::acceptCon()
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
        sin_size = sizeof their_addr;

        /**
         * Create a new TCPSocket from the accepted connection
         */
        TCPSocket* clientSocket = new TCPSocket(accept(_boundSocketDesc, (struct sockaddr *) &their_addr, &sin_size), their_addr);


        // IF there was a failure accepting
        if (clientSocket->getSocketDescriptor() == -1)
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

            return clientSocket;

        }

    } // END WHILE

}


///
/// Function name: checkBoundSocket
/// Description: Verifies that the bound socket is indeed bound.
/// Parameters: n/a
/// Returns: bool denoting its success or failure
///
void TCPServerSocket::checkBoundSocket()
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
void TCPServerSocket::checkConnSocket()
{
    // IF our bound socket is not set
    if(_currConnSocketDesc == NOT_CONNECTED)
        throw runtime_error("Error with socket. _currConnSocketDesc is not connected!");

}

///
/// Function to return the address in a sockaddr structure,
/// regardless of IPv4 or IPv6.
///
void*TCPServerSocket::get_in_addr(struct sockaddr *sa)
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
void*TCPServerSocket::get_in_port(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_port);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_port);
}
