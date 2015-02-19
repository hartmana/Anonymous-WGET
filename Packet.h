#ifndef PACKET_H
#define PACKET_H

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
/// Class to define what a Packet consists of.
///
class Packet {

private:
    uint16_t _version;
    uint16_t _length;

    char *_payload;

    char *_encodedPacket;


public:

    // Constructor to wrap a socket file descriptor with a TCPSocket object
    Packet(uint16_t version, char& payload);

    // Destructor
    ~Packet();

    void encodePacket();

    void decodePacket();

    inline uint16_t getVersion() { return _version; }

    inline uint16_t getLength() { return _length; }

    inline char& getPayload() { return _payload; }

    inline char& getEncodedPacket() { return _encodedPacket; }


};

#endif
