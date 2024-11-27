#pragma once

#ifndef __PROJECT_NETWORK_LAYER_H_
#define __PROJECT_NETWORK_LAYER_H_

#include <omnetpp.h>
#include "../Utils/FileIO.h"
#include "../Utils/Logger.h"

using namespace omnetpp;

class NetworkLayer
{
private:
    std::queue<PacketInfo*> Packets;
public:
    void LoadPackets(std::string NodeName);
    PacketInfo* FromNetworkLayer();
    void ToNetworkLayer(std::string payload,int seqNum);
};

#endif
