#pragma once
#include <omnetpp.h>
#include <bitset>
#include "../Frame/Frame_m.h"

using namespace omnetpp;

const unsigned short GX = 0x07; // 0b111

void ApplyByteStuffing(std::string payload,Frame* frame){
    std::vector<char> newPayload;
    newPayload.push_back('$');
    for (char c : payload) {
        if (c == '/' || c == '$')
            newPayload.push_back('/');
        newPayload.push_back(c);
    }
    newPayload.push_back('$');
    std::string stuffedPayload(newPayload.begin(), newPayload.end());
    frame->setPayload(stuffedPayload.c_str());
}

std::string UnApplyByteStuffing(Frame* frame){
    std::vector<char> newPayload;
    bool isFlag = true;
    for (char c : std::string(frame->getPayload())) {
        if ((c == '/') && isFlag){
            isFlag=false;
            continue;
        }
        newPayload.push_back(c);
        isFlag=true;
    }
    std::string payload(newPayload.begin()+1, newPayload.end()-1);
    return payload;
}

void ApplyCRC(Frame* frame){
    std::string payload = frame->getPayload();
    unsigned char crc = 0xFF;
    for (char c : payload) {
        crc ^= (unsigned char)c;
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x80)
            {
                crc = (crc << 1) ^ GX;
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    frame->setTrailer(reinterpret_cast<const char*>(&crc));
}

bool checkCRC(Frame* frame) {
    std::string payload = frame->getPayload();
    unsigned char sentCRC = frame->getTrailer()[0];

    unsigned char crc = 0xFF;
    for (char c : payload) {
        crc ^= (unsigned char)c;
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ GX;
            } else {
                crc <<= 1;
            }
        }
    }
    return (crc == sentCRC);
}
