#include "NetworkLayer.h"

void NetworkLayer::LoadPackets(std::string NodeName)
{
    std::string fileToRead = "input" + std::to_string(int(NodeName[NodeName.size() - 1] - '0'));
    Packets = ReadSenderTxt(fileToRead);
}

PacketInfo* NetworkLayer::FromNetworkLayer(){
    if (Packets.empty()) return nullptr;
    PacketInfo* packet = Packets.front();
    Packets.pop();
    return packet;
}

void NetworkLayer::ToNetworkLayer(std::string payload,int seqNum){
    EV << "Uploading payload=" + payload + " and seq_num = [" + std::to_string(seqNum) + "] to the network layer" <<std::endl;
}
