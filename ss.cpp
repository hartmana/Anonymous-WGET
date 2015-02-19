//
//  ss.cpp (stepping stone)
//  cs457
//
//  Created by Aaron Hartman on 9/5/14.
//  Copyright (c) 2014 Revalent Corp. All rights reserved.
//
#define CHAIN_FILE "chaingang.txt"
#define OUTPUT_DIR "/s/bach/j/under/ahrtmn/awget/awget_tmp/"
//#define OUTPUT_DIR "/Users/ahrtmn/Projects/awget/awget_tmp/"

#include "TCPSocket.h"
#include "TCPServerSocket.h"
#include "StringUtils.h"
#include "NetworkUtils.h"

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <cstdlib>
#include <unordered_map>
#include <sys/stat.h>
#include <dirent.h>
#include <thread>



using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::string;
using std::stringstream;
using std::ifstream;
using std::ofstream;
using std::unordered_map;
using std::to_string;
using std::thread;
using std::bind;



///
/// Print the correct usage in case of user syntax error.
///
int usage(char *arg0)
{
    
    cout << "Usage: " << arg0 << " [-c chainfile] <URL>" << endl;
    cout << "Chain file must be specified if there is no chainfile local to awget." << endl;
    cout << "Chain file, if specified, must be before the URL." << endl;
    cout << "URL argument mandatory to specify the file for awget to retrieve." << endl;

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
/// Function name: writeChainFile
/// Description: Writes out to file the contents of a passed in unordered map
///              in the format of:
///                
///              <SSNum>
///              <SSaddr, SSport>
///              ...
///
/// Parameters: char string containing the path name the file is to be written to
///             unordered_map reference containing SS data to be written.
/// Returns: void
///
void writeChainFile(const char* chainFilePath, const unordered_map<int, vector<string> >& chainMap)
{
    ofstream ostream(chainFilePath, ofstream::trunc);

    ostream << chainMap.size() << endl;

    for(unordered_map<int, vector<string> >::const_iterator iter = chainMap.begin(); iter != chainMap.end(); ++iter)
        ostream << iter->second.at(0) << "," << iter->second.at(1) << endl;
}


///
/// Function to read the chain gang file as an array,
/// split it by entry and select one at random, returning
/// a connection to the chosen stepping stone. The chain
/// gang file data will be modified with the chosen entry removed,
/// the count updated, and the array ready to be sent to the next
/// SS.
///
/// Parameters: char* to the file data with each line delimited by ':'.
///             char* to be filled with the URL from the passed in data. [MUST BE FREED]
///             unsigned int& to be modified with total size of returned data in requestData.
/// Returns: TCPSocket to the chosen SS.
///          NULL reference if this SS is the last in the list
///
TCPSocket* getNextSSCon(char* requestData, char* &urlString, unsigned int& resultLength)
{
    
    StringUtils stringUtils;

    // split the chain file data by line
    vector<string> chainFileLines = stringUtils.split(requestData, ':');

    // get the URL string for the caller
    unsigned int urlStringLength = chainFileLines.at(0).length() + 1;
    urlString = new char[urlStringLength];
    memset(urlString, '\0', sizeof(char) * urlStringLength);
    
    strcpy(urlString, chainFileLines.at(0).c_str());

    // Number of SS entries
    int numSS = atoi(chainFileLines.at(1).c_str());

    unordered_map<int, vector<string> > ssMap;

    unsigned int keyIndex = 0;
    

    // FOR every line stored in the vector, ignoring the URL and the number of entries
    for(vector<string>::const_iterator lineIter = (chainFileLines.begin() + 2); lineIter != chainFileLines.end(); ++lineIter)
    {

        /**
         * Every line in the file will be split by comma,
         * seperating the IP address from the Port number
         * and storing them in a 2 element vector for each line.
         * These vectors will be placed in an unordered_map. 
         */
        vector<string> res = stringUtils.split(*lineIter, ',');

        ssMap.insert(std::make_pair(keyIndex++, res));

    }

    // IF there are no more entries
    if(numSS == 0)
        return NULL;        // allow the calling program to handle this situation
                            // chain file string will be unmodified
    
    /**
     * Select a random SS, make a connection to it, and 
     * erase the selection from our SS hash map.
     */
    srand(time(NULL));

    int randKey = rand() % keyIndex;

    vector<string>* randSS = &ssMap[randKey];

    TCPSocket* randSSCon = new TCPSocket((*randSS).at(0), (*randSS).at(1));

    ssMap.erase(randKey);


    /**
     * Rebuild the request data with the modified SS count
     * and the updated SS list before returning.
     */ 
    memset(requestData, '\0', (strlen(requestData) + 1));

    int requestDataIndex = 0;   // keep track of our position while adding back info
    
    // Copy the URL back in
    for(unsigned int i = 0; i < strlen(urlString); ++i)
        requestData[requestDataIndex++] = urlString[i];

    requestData[requestDataIndex++] = ':';    

    
    /**
     * Update the SS count and add it back to the request data
     */
    string numFormat;
    numSS = numSS - 1;
    numFormat = to_string(numSS);
    strcat(requestData, numFormat.c_str());
    requestDataIndex++;
    
    requestData[requestDataIndex++] = ':';    
    
    
    // FOR every entry in the stepping stone map
    for(unordered_map<int, vector<string> >::const_iterator mapIter = ssMap.begin(); mapIter != ssMap.end(); ++mapIter)
    {

        // FOR every char in the IP
        for(string::const_iterator ipIter = mapIter->second.at(0).begin(); ipIter != mapIter->second.at(0).end(); ++ipIter)
            requestData[requestDataIndex++] = *ipIter;

        // separate IP and port
        requestData[requestDataIndex++] = ',';

        // FOR every char in the port
        for(string::const_iterator portIter = mapIter->second.at(1).begin(); portIter != mapIter->second.at(1).end(); ++portIter)
            requestData[requestDataIndex++] = *portIter;

        // separate each SS entry
        requestData[requestDataIndex++] = ':';

    }

    requestData[requestDataIndex++] = '\0';

    resultLength = requestDataIndex;
    
    return randSSCon;

}

///
/// Function to be ran in its own thread once an incoming connection is received.
/// handleConnection will determine if it is the last SS on the list. If it is,
/// it will execute the wget command to retrieve the specified url (once received), and
/// if not will select the next SS to forward the data to.
///
/// Parameters: TCPSocket* to retrieve the request from, and if this is the last SS
///             destination will send the retrieved file back with this connection.
/// Returns: void
///
void handleConnection(TCPSocket* incSocket)
{

    // Chain file data passed by the incoming connection, memory handled by the socket class
    unsigned int bytesRead = 0;
    unsigned char* chainFileData = incSocket->read(bytesRead);

    // Pointer to contain the stripped out url string from the chain file data
    char *urlString = NULL;

    unsigned int chainFileDataLength = 0;

    /*  Get the next connection (if there is one), also obtaining the url and length of data in case it needs to be resent  */
    TCPSocket* nextCon = getNextSSCon(reinterpret_cast<char *>(chainFileData), urlString, chainFileDataLength);


    // IF getNextSSCon determined that there are no more entries on the chain list
    if(nextCon == NULL)
    {

        /**
         * Make sure there is a directory to download the file to
         */
        struct stat st = {0};

        if (stat(OUTPUT_DIR, &st) == -1) 
        {
            mkdir(OUTPUT_DIR, 0700);
        }


        /**
         * Build the wget command string
         */
        string wget("wget ");
        wget.append("--directory-prefix=");
        wget.append(OUTPUT_DIR);
        wget.append(" ");
        wget.append(urlString); 
        


        // IF the wget succeeds
        if(system(wget.c_str()) == 0)
            cout << "Url retrieved: " << urlString << endl;



        /**
         * Get the name of the file we just saved, knowing that it is
         * the only file in the directory that the SS created.
         */
        DIR *dir;
        struct dirent *ent;
        vector<string> dirEntries;


        // IF opening the directory succeeds
        if ((dir = opendir (OUTPUT_DIR)) != NULL) 
        {
        
            // WHILE there are more entries in the directory
            while((ent = readdir(dir)) != NULL) 
            {
                dirEntries.push_back(ent->d_name);  // store them
            }

            closedir (dir);
        } 
        // ELSE if the directory could not be opened
        else 
        {
            perror ("Could not open directory for saved awget file.");
            exit(EXIT_FAILURE);
        }



        // string containing the path to the saved file to be read and sent
        string filePath(OUTPUT_DIR); 
        filePath.append(dirEntries.at(2));  // entries 0 and 1 are '.' and '..'.  
    


        /**
         * Read in the file to an array
         */        

        // Open the file in binary
        ifstream fileStream(filePath, ifstream::in | ifstream::binary);


        unsigned int fileSize = 0;

        // IF the file stream opened successfully
        if (fileStream) 
        {
            // get length of file:
            fileStream.seekg (0, fileStream.end);
            fileSize = fileStream.tellg();
            fileStream.seekg (0, fileStream.beg);
        }

        unsigned char fileData[fileSize];

        // read data as a block:
        fileStream.read (reinterpret_cast<char *>(fileData), fileSize);
        


        /**
         * Preparing to send the requested file back to the calling awget program; 
         * three lengths need to be prepended to the first 6 bytes of the array:
         * 0-1: Length of the total array to be sent  
         * 2-3: Length of the file name 
         * 4-5: Length of the file data itself
         */
        unsigned int fileNameLength = strlen(dirEntries.at(2).c_str());
        unsigned int payloadSize = (7 + fileNameLength + fileSize);        // encoded lengths + payload + null
        unsigned char payload[payloadSize];


        uint16_t encodedSize = (payloadSize - 2);   // total payload array size minus the length we are about to encode

        payload[0] = (encodedSize >> 8) & 0x00FF;
        payload[1] = (encodedSize & 0x00FF);


        /**
         * Encode the length of the file name
         */
        uint16_t nameLength = static_cast<uint16_t>(fileNameLength); 

        payload[2] = (nameLength >> 8) & 0x00FF;
        payload[3] = (nameLength & 0x00FF);


        /**
         * Encode the file size
         */
        uint16_t fileDataLength = static_cast<uint16_t>(fileSize);

        payload[4] = (fileDataLength >> 8) & 0x00FF;
        payload[5] = (fileDataLength & 0x00FF);



        /**
         * Copy the file name and file data to the remainder of the array
         */
        int i = 6; // index starts after encoded lengths

        // FOR every character in the file name
        for(unsigned int j = 0; j < fileNameLength; ++j)
            payload[i++] = dirEntries.at(2)[j];       // encode it


        // FOR every byte of file data
        for(unsigned int k = 0; k < fileSize; ++k)
            payload[i++] = fileData[k];          // encode it

        payload[payloadSize] = '\0';


        cout << "Writing file.. " << endl;

        incSocket->write(payload, static_cast<size_t>(payloadSize));

        cout << "File written to socket" << endl;



    }
    else
    {
        cout << "Next connection: " << endl;
        cout << "IP: " << nextCon->getClientIpAddress() << "\tPort: " << nextCon->getClientPort() << endl;

        cout << "Sending SS request.." << endl;

        unsigned char* packedSSData = prependSize(chainFileData, chainFileDataLength);

        nextCon->write(packedSSData, chainFileDataLength);

        cout << "Waiting for file to be passed back.. " << endl;

        bytesRead = 0;
        unsigned char* completeFileData = nextCon->read(bytesRead);

        unsigned char* packedCompleteData = prependSize(completeFileData, bytesRead);

        cout << "File data received!" << endl << "Sending file to previous connection.." << endl;

        incSocket->write(packedCompleteData, bytesRead);

        delete packedCompleteData;
    }


    if(urlString != NULL)
        delete urlString;
    
}


///
/// Main method to check for arguments and initiate the 
/// retrieval of the specified URL.
///
int main(int argc, char * const argv[])
{
    int port = 0;   // port the server will listen to

    /** Ascii representation of the user-entered option **/
    int c;
    
    // reset the opterr global
    opterr = 0;
    
    
    // WHILE there are more options, check for c
    while (optind < argc)
    {
        if ((c = getopt(argc, argv, "p:")) != -1)
        {
        
            switch (c)
            {
                case 'p':
                    
                    port = atoi(optarg);
                    break;
                default:
                    cout << "Default Case. Printing usage statement and exiting.";
                    usage(*argv);
                    break;
                    
            }// END SWITCH
        }// END IF

    }// END WHILE

    
    cout << "Welcome to 'a' Stepping Stone!" << endl;
    cout << "--------------------------------" << endl;

    /**
     ***********REMOVE CONSTRUCTOR CALL*************
     */
    TCPServerSocket serverSocket("65534");

    cout << "Listening on:" << endl;
    cout << "IP: " << serverSocket.getHostIpAddress() << "\tPort: " << serverSocket.getPort() << endl;

    while(true)
    {
        TCPSocket* incSocket = serverSocket.acceptCon();    

        thread worker(bind(handleConnection, incSocket));

        worker.detach();
    }
    

    return 0;
}
