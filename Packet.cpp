//
//  Packet.cpp
//
//  Created by Aaron Hartman on 3 OCT 14.
//  Copyright (c) 2014. All rights reserved.
//

#include "Packet.h"


Packet::Packet(uint16_t version, char& payload)
{
    _version = version;
    _payload = payload;
    _length = strlen(_payload);
    _encodedPacket = NULL;
}

Packet::~Packet()
{
    if(_payload != NULL)
    {
        delete _payload;
        _payload = NULL;
    }

    if(_encodedPacket != NULL)
    {
        delete _encodedPacket;
        _encodedPacket = NULL;
    }
}

///
/// Function name: encodePacket
/// Description: encodes, or converts, all packet data into binary big endian
/// Parameters: n/a
/// Returns: void
///
void Packet::encodePacket()
{

}

///
/// Function name: decodePacket
/// Description: decodes, or converts, all packet data from binary big endian to
/// Parameters: n/a
/// Returns: void
///
void Packet::decodePacket()
{

}
