#ifndef PACKETSTRUCT_H
#define PACKETSTRUCT_H

#define URL_LENGTH 140

#include <cstdint>

struct PacketStruct {
    uint16_t version;
    uint16_t length;
    char message[URL_LENGTH];
};

#endif