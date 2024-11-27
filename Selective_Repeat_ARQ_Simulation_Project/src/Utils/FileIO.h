#pragma once
#include <omnetpp.h>
#include <fstream>
#include <vector>
#include <queue>

using namespace omnetpp;

void ReadCoordinatorTxt(std::string& senderNode,std::string& startTime){
    std::ifstream Fin;
    Fin.open("../input/Coordinator.txt");
    if(Fin.is_open())
    {
        Fin >> senderNode >> startTime;
    }
    else
    {
        EV<<"Coordinator.txt is not found 404" <<std::endl;
    }
    Fin.close();
}

struct PacketInfo {
    bool modification;
    bool loss;
    bool duplication;
    bool delay;
    std::string message;
};

std::queue<PacketInfo*> ReadSenderTxt(std::string fileName) {
    std::queue<PacketInfo*> packets;
    std::ifstream Fin;
    Fin.open("../input/" + fileName + ".txt");
    
    if (Fin.is_open()) {
        std::string line;

        while (std::getline(Fin, line)) {
            if (line.length() < 6) continue;
            std::string flag = line.substr(0, 4);
            bool modification = (flag[0] == '1');
            bool loss = (flag[1] == '1');
            bool duplication = (flag[2] == '1');
            bool delay = (flag[3] == '1');

            std::string message = line.substr(5);

            PacketInfo* packet = new PacketInfo({modification, loss, duplication, delay, message});
            packets.push(packet);
        }
    } 
    else {
        EV << fileName + ".txt is not found 404" << std::endl;
    }
    Fin.close();
    return packets;
}

