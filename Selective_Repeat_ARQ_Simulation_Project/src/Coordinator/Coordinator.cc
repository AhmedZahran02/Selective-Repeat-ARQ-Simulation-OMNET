#include "Coordinator.h"

Define_Module(Coordinator);

void Coordinator::initialize()
{
    std::string senderNode, startTime;
    ReadCoordinatorTxt(senderNode, startTime);
    Frame* msg = new Frame();
    msg->setKind(0);
    msg->setName("Coordinator");
    msg->setFrameType(-1);
    if(senderNode == "0")
    {
        sendDelayed(msg, stoi(startTime), "node1");
    }
    else if (senderNode == "1")
    {
        sendDelayed(msg, stoi(startTime), "node2");
    }
}

void Coordinator::handleMessage(cMessage *msg)
{
}
