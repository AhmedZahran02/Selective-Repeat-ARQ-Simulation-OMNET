#pragma once
#include <omnetpp.h>

std::string toBinary(const char* data) {
    std::string binary = "";
    while (*data) {
        binary += std::bitset<8>(*data).to_string();
        data++;
    }
    return binary;
}
