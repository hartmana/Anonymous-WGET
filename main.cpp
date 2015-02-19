//
//  main.cpp
//  cs457
//
//  Created by Aaron Hartman on 9/5/14.
//  Copyright (c) 2014 Revalent Corp. All rights reserved.
//

#include "TCPSocket.h"
#include "TCPServerSocket.h"

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>


using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::string;
using std::stringstream;




///
/// Print the correct usage in case of user syntax error.
///
int usage(char *arg0)
{
    
    cout << "Usage: " << arg0 << " [-p] [-s] [-h]" << endl;
    cout << "All commands are optional. No commands specified will result in  " << endl;
    cout << "the program running in chatServer mode." << endl << endl;
    cout << "Optional arguments: " << endl;
    cout << arg0 << " -p [port number] -s [IP address]" << endl;
    cout << "Specifying a port and ip address the program will attempt to connect" << endl;
    cout << "to the specified location (assumes another instance of this program" << endl;
    cout << "is listening at the given location)." << endl << endl;
    cout << arg0 << " -h [help]" << endl;
    cout << "Specifying the help flag will cause the program to print a help message." << endl;
    
    exit(EXIT_FAILURE);
}

///
/// Function to check that a specified IP address only contains
/// integers and '.' characters.
///
bool checkIP(char *ip)
{
    if(ip == NULL)
        return false;
    
    for(unsigned int i = 0; i < strlen(ip); ++i)
    {
        if(!isdigit(ip[i]) && (ip[i] != '.'))
        {
            cout << "IP Address must ONLY contain numbers and the character '.'!";
            return false;
        }
    }
    
    return true;
}

///
/// Function to check the message length of a char array to make sure it fits
/// within the packet constraints.
///
bool checkMessageLen(char *message)
{
    if(message == NULL)
        return false;

    return strlen(message) <= MESSAGE_LENGTH;
}


///
/// Function to check the close condition (input of X) and close the program
/// if appropriate.
///
void checkClose(char *message)
{
    if(strcmp(message, "X") == 0)
    {
        cout << "Exiting.." << endl;
        exit(0);
    }
}

///
/// Main method to check for arguments and run the chat program
/// in either chatServer or chatClient mode depending on the specified
/// arguments.
///
int main(int argc, char * const argv[])
{
    /**  Flags to be set by the getopt routine    **/
    bool portSet = false;
    bool ipSet = false;
    bool helpSet = false;
    
    /**  Pointers to the option arguments    **/
    char *portNum = NULL;
    char *ipAdd = NULL;
    
    
    /** Ascii representation of the user-entered option **/
    int c;
    
    // reset the opterr global
    opterr = 0;
    
    
    // WHILE there are more options, check for p, s, and h
    while ((c = getopt(argc, argv, "p:s:h")) != -1)
    {
        
        switch (c)
        {
            case 'p':
                portSet = true;
                portNum = optarg;
                
                break;
            case 's':
                ipSet = true;
                ipAdd = optarg;
                
                break;
            case 'h':
                helpSet = true;
                
                break;
                // Missing argument case
            case '?':
                
                // IF it was the port or IP flags and an argument was missing
                if (optopt == 'p' || optopt == 's')
                {
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                }
                else if (isprint (optopt))
                {
                    fprintf (stderr, "Unknown option `-%c'.\n\n", optopt);
                }
                else
                {
                    fprintf (stderr,
                             "Unknown option character `\\x%x'.\n",
                             optopt);
                }
                
                usage(*argv);   // print usage and exit
                break;
                
            default:
                cout << "Default Case. Printing usage statement and exiting.";
                usage(*argv);
                break;
                
        }
        
    }
    
    // IF an IP value was given but was null or did not meet the required min length
    if(ipSet && (ipAdd == NULL || strlen(ipAdd) < 7))
    {
        usage(*argv);   // tell them what they did wrong
    }
    
    
    
    printf ("portSet = %d, ipSet = %d, helpSet = %d\n",
            portSet, ipSet, helpSet);
    
    
    printf("Port: %s\tIP: %s\n", portNum, ipAdd);
    

    
    for (int index = optind; index < argc; index++)
        printf ("Non-option argument %s\n", argv[index]);
    
    
    
    if(argc > 2 && !checkIP(ipAdd))
    {
        cerr << "Invalid IP address specified!" << endl;
        usage(*argv);   // exit point
    }
    
    
    
    cout << "Welcome to Chat!" << endl;
    

    char message[MESSAGE_LENGTH * 2];
    
    switch(argc)
    {
        case 1: {
            TCPServerSocket serverSocket;
            
            cout << "Server is listening on: " << serverSocket.getHostIpAddress() << ":" << serverSocket.getPort() << endl;
            
            serverSocket.listenForCon();
            
            cout << "Waiting to Accept connection.." << endl;
            
            TCPSocket activeConnection = serverSocket.acceptCon();
            
            cout << "Connected to [" << activeConnection.getClientIpAddress() << ":" << activeConnection.getClientPort() << "]" << endl;
            cout << "Input of 'X' to quit." << endl;
            
            

            struct PacketStruct *pRec;
            
            struct PacketStruct pSend;


            memset(&pSend, 0, sizeof pSend);
            
            while(true)
            {
                
                activeConnection.readPacket();
                
                pRec = activeConnection.getPacket();
                
                cout << endl << "Friend: " << pRec->message << endl;
                
                while(true)
                {
                    cout << "You: ";
                    
                    cin.getline(message, sizeof message);

                    if(!checkMessageLen(message))
                    {
                        cout << "Error: Input too long" << endl;
                        continue;
                    }
                    else
                    {
                        // check to see if we should exit
                        checkClose(message);

                        strcpy(pSend.message, message);
                        break;
                    }
                }

                pSend.version = 457;
                pSend.length = strlen(pSend.message) + 1;
                
                activeConnection.writePacket(&pSend);
                
                
            }
            
            break;
        }
        case 2: {
            usage(*argv);
            
            break;
        }
        case 5: {
            
            TCPSocket clientSocket(ipAdd, portNum);
            
            cout << "Connected to: " << clientSocket.getClientIpAddress() << ":" << clientSocket.getClientPort() << endl;
            
            cout << "You send message first. ('X' to quit)" << endl;
            
            

            struct PacketStruct *pRec;
            struct PacketStruct pSend;

            memset(&pSend, 0, sizeof pSend);


            
            while(true)
            {
                cout << "You: ";
                
                cin.getline(message, sizeof message);

                if(!checkMessageLen(message))
                {
                    cout << "Error: Input too long" << endl;
                    continue;
                }
                else
                {
                    // check to see if we should exit
                    checkClose(message);

                    strcpy(pSend.message, message);
                }
                
                pSend.version = 457;
                pSend.length = strlen(pSend.message) + 1;


                
                clientSocket.writePacket(&pSend);
                
                clientSocket.readPacket();
                
                pRec = clientSocket.getPacket();
                
                cout << endl << "Friend: " <<  pRec->message << endl;
            }
            
            
            break;
        }
        default: {
            usage(*argv);
            
            break;
        }
            
    }
    
    return 0;
}
