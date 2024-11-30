#pragma once

#ifndef __PROJECT_NODE_H_
#define __PROJECT_NODE_H_

#include <omnetpp.h>
#include "../Utils/FileIO.h"
#include "../Utils/Framming.h"
#include "../Errors/ErrorSimulation.h"
#include "../Utils/General.h"
#include "../Utils/Logger.h"
#include "../Frame/Frame_m.h"
#include "../NetworkLayer/NetworkLayer.h"
#include <utility>

using namespace omnetpp;

class Node : public cSimpleModule
{
private:
    int WindowSize;
    int MAX_SEQ;
    double TO;
    double PT;
    double TD;
    double ED;
    double DD;
    double LP;

    int seqNumber;

    int startIndex;
    int endIndex;
    int currentWindowSize;
    std::vector<Frame*> SeqList;
    std::vector<double> timeouts;
    std::vector<std::pair<std::pair<std::string,int>,bool>> payloadList;

    std::vector<std::vector<bool>> errors;

    NetworkLayer networkLayer;
  protected:
    void reFillTheWindow();
    void MoveRecieverWindow(Frame* frame);


    void handleRecieveData(Frame* frame);
    void handleACK(Frame* frame);
    void handleNACK(Frame* frame);
    void handleTimeout(Frame* frame);
    void sendFrame(Frame *frame);

    void IncrementSeqNum(int& seqNum);
    void IncrementWindowIndex(int &index);


    void parametersInitialization();
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

#endif
