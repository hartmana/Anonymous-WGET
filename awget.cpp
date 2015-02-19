//
//  main.cpp
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

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <cstdlib>
#include <unordered_map>


using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::string;
using std::stringstream;
using std::ifstream;
using std::ofstream;
using std::unordered_map;
using std::iterator;



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
/// Function to read the chain gang file, select a random SS
/// and remove that SS from the file.
///
/// Parameters: char* to the file
/// Returns: TCPSocket to the chosen SS
///
TCPSocket& getNextSSCon(const char* chainFilePath)
{
    ifstream fileStream(chainFilePath);
    
    // StringUtils for split functionality
    StringUtils stringUtils;

    unordered_map<int, vector<string> > ssMap;

    char line[100];
    
    // Get the first line, which should contain the num of stepping stones
    fileStream.getline(line, sizeof(line));

    unsigned int numSS = static_cast<unsigned int>(atoi(line));


    unsigned int keyIndex = 0;

    // FOR every SS we are expecting
    for(; keyIndex < numSS; ++keyIndex)
    {

        /**
         * Every line in the file will be split by comma,
         * seperating the IP address from the Port number
         * and storing them in a 2 element vector for each line.
         * These vectors will be placed in an unordered_map. 
         */

        fileStream.getline(line, sizeof(line));

        string ssEntry(line);

        vector<string> res = stringUtils.split(ssEntry, ',');

        ssMap.insert(std::make_pair(keyIndex, res));

    }

    /**
     * Now that we have all of the information we only need to 
     * select a random SS and return a connection to it.
     */
    srand(static_cast<unsigned int>(time(NULL)));

    int randKey = rand() % keyIndex;

    vector<string>* randSS = &ssMap[randKey];


    TCPSocket* randSSCon = new TCPSocket((*randSS).at(0), (*randSS).at(1));
    
    ssMap.erase(randKey);


    writeChainFile(chainFilePath, ssMap);

    return *randSSCon;

}

///
/// Function to read the chain gang file, delimit new lines by ':',
/// and return the file as a char array. 
///
/// Parameters: char* to the file
/// Returns: char* of the file in its delimited form.
///
unsigned char* getDataAsArray(char *chainFilePath, char *url, unsigned int& resultLength)
{
    string line;
    unsigned int charCount = 0;
    unsigned int lineCount = 0;
    vector<string> res;
    
    ifstream fileStream(chainFilePath);

    // WHILE more lines in the file    
    while(getline(fileStream, line))
    {

        // update the count
        charCount += line.length();

        // used for delimiting space
        ++lineCount;

        res.push_back(line);
    }

    /*  Char array to contain all data to be sent to the stepping stones.
     *  Length of file + Length of URL + Num of Delimiting chars + 2 for the encoded size + 1 for the null char  */   
    resultLength = static_cast<unsigned int>(charCount + lineCount + strlen(url) + 3);     // make sure the caller knows the final size
    unsigned char* fileAsArray = new unsigned char[resultLength];

    string urlString(url);

    int i = 2;

    for(string::iterator urlIter = urlString.begin(); urlIter != urlString.end(); ++urlIter)
        fileAsArray[i++] = *urlIter;

    fileAsArray[i++] = ':';

    // FOR every line that was in the file
    for(vector<string>::iterator it = res.begin(); it != res.end(); ++it)
    {
        // Add the line to the encoding
        for(unsigned int j = 0; j < (*it).length(); ++j)
        {
            fileAsArray[i++] = (*it)[j];
        }


        fileAsArray[i++] = ':';

    }

    fileAsArray[i++] = '\0';

    // Length of the data less the encoded length
    uint16_t length = i - 2;

    /**
     * Add the length to the first two bytes of the string before 
     * appending the url and chain file data.
     */
    fileAsArray[0] = (length >> 8) & 0x00FF;
    fileAsArray[1] = (length & 0x00FF);

    return fileAsArray;
}


///
/// Main method to check for arguments and initiate the 
/// retrieval of the specified URL.
///
int main(int argc, char * const argv[])
{
    char *urlOfFile = NULL; 
    bool hasURL = false;

    char *chainFilePath = NULL;
    bool hasChainFile = false;
    
    /** Ascii representation of the user-entered option **/
    int c;
    
    // reset the opterr global
    opterr = 0;
    
    
    // WHILE there are more options, check for c
    while (optind < argc)
    {
        if ((c = getopt(argc, argv, "c:")) != -1)
        {
        
            switch (c)
            {
                case 'c':
                    hasChainFile = true;
                    
                    chainFilePath = new char[strlen(optarg) + 1];
                    strcpy(chainFilePath,optarg);
                    break;
                                  
                default:
                    cout << "Default Case. Printing usage statement and exiting.";
                    usage(*argv);
                    break;
                    
            }// END SWITCH
        }// END IF
        // ELSE we have a non option arg
        else
        {
            urlOfFile = argv[optind++];
            hasURL = true;
        }

    }// END WHILE
    

    // IF no chainfile was specified - use the default
    if(!hasChainFile)
    {
        chainFilePath = new char[14];
        strcpy(chainFilePath, CHAIN_FILE);  
    }

    // IF no URL was specified
    if(!hasURL)
    {
        usage(*argv);   // must has URL
    }
    
    cout << "Welcome to awget!" << endl;
    cout << "--------------------------------" << endl;
    cout << endl << "Chain file to be read: " << chainFilePath << endl;
    cout << "URL to be retrieved: " << urlOfFile << endl;

    

    // Get a socket to the next stepping stone
    TCPSocket ssSocket = getNextSSCon(chainFilePath);

  
    
    /**
     * Get the file into string form with each line delimited by ':'
     */
    unsigned int fileStringLength = 0; 
    unsigned char* fileString = getDataAsArray(chainFilePath, urlOfFile, fileStringLength);
     
    
    // Send the file to the stepping stone
    ssSocket.write(fileString, fileStringLength);


    
    /**
     * Wait and receive the returned file from the stepping stones
     */
    unsigned int bytesRead = 0;
    unsigned char *encodedFile = ssSocket.read(bytesRead);

    cout << "FILE READ!" << endl;
    
    // Decode the file name length
    uint16_t fileNameLength = encodedFile[0] << 8;
    fileNameLength |= encodedFile[1];
    cout << "fileNameLength: " << fileNameLength << endl;


    // Decode the file size
    uint16_t fileSize = encodedFile[2] << 8;
    fileSize |= encodedFile[3];
    cout << "fileSize: " << fileSize << endl;


    // construct a string of the file name, ignoring the first 4 bytes
    string fileName(reinterpret_cast<char *>((encodedFile + 4)), fileNameLength);  

    cout << "Filename: " << fileName << endl;


    ofstream fileStream(fileName, ofstream::out | ofstream::binary);

    // Write the file out ignoring the encoded sizes and the file name itself
    fileStream.write(reinterpret_cast<char *>((encodedFile + fileNameLength + 4)), fileSize);

    fileStream.close();
    
    if(chainFilePath != NULL)
        delete chainFilePath;
    if(fileString != NULL)
        delete fileString;

    return 0;
}
