#ifndef NETWORKUTILS_H
#define NETWORKUTILS_H


#include <iostream>
#include <stdint.h>		// uint16_t

using std::cout;
using std::endl;

///
/// Function name: prependSize
/// Description: Creates a char array 2 bytes larger than the referenced array, encodes
///				 the length of the array into those first two bytes and then
///				 returns it. 
///
/// Parameters: data: Reference to the char array to be encoded.
///				dataSize: size of the referenced array in case it is not null terminated.
///						  Will be increased to reflect the added 2 bytes.
///
/// Returns: Pointer to a heap char array containing the result.
///
unsigned char* prependSize(const unsigned char* data, unsigned int& dataSize)
{

	unsigned char* encodedArray = new unsigned char[dataSize + 2];
	uint16_t size = static_cast<uint16_t>(dataSize);

	encodedArray[0] = ((size >> 8) & 0x00FF);
    encodedArray[1] = (size & 0x00FF);

    uint16_t messageLength = encodedArray[0] << 8;
    messageLength |= encodedArray[1];

    cout << "encoded message size: " << messageLength << endl;


    // FOR every byte of the original data
    for(unsigned int i = 0; i < dataSize; ++i)
    	encodedArray[i + 2] = data[i];

    dataSize = dataSize + 2;

    return encodedArray;
}


#endif