#pragma once

#ifndef __PROJECT_COORDINATOR_H_
#define __PROJECT_COORDINATOR_H_

#include <omnetpp.h>
#include "../Utils/FileIO.h"
#include "../Frame/Frame_m.h"

using namespace omnetpp;

class Coordinator : public cSimpleModule
{
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

#endif
