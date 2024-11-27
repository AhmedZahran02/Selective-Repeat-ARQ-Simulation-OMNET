#pragma once
#include <omnetpp.h>
#include "../Frame/Frame_m.h"

std::string modifyPayload(Frame* frame) {
    std::string payload = frame->getPayload();

    std::vector<unsigned char> bytePayload(payload.begin(), payload.end());

    srand(time(0));

    int byteIndex = rand() % bytePayload.size();
    unsigned char &byte = bytePayload[byteIndex];

    int bitIndex = rand() % 8;
    byte ^= (1 << bitIndex);

    std::string modifiedPayload(bytePayload.begin(), bytePayload.end());
    frame->setPayload(modifiedPayload.c_str());

    return std::to_string((byteIndex * 8) + bitIndex);
}

std::vector<std::string> ApplyError(Frame *frame, bool loss, bool modification,
        bool delay, bool duplication) {
    std::vector<std::string> results(4);
    results[0] = "-1";
    results[1] = "No";
    results[2] = "0";
    results[3] = "0";

    if (loss) {
        results[1] = "Yes";
        return results;
    }
    if (modification) // Modification error
    {
        results[0] = modifyPayload(frame);
    }
    if (delay)
    {
        results[3]= "1";
    }
    if (duplication)
    {
        results[2]="1";
    }
    return results;
}
