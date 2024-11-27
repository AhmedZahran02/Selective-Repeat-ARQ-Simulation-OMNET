#include "Node.h"

Define_Module(Node);

const std::string OUTPUTFILEPATH = "../output/output.txt";

void Node::parametersInitialization() {
    WindowSize = par("WindowSize").intValue();
    MAX_SEQ = par("MAX_SEQ").intValue();
    TO = par("TO").doubleValue();
    PT = par("PT").doubleValue();
    TD = par("TD").doubleValue();
    ED = par("ED").doubleValue();
    DD = par("DD").doubleValue();
    LP = par("LP").doubleValue();

    seqNumber = 0;
    startIndex = 0;
    endIndex = (MAX_SEQ + 1) / 2 - 1;
    currentWindowSize = 0;
    SeqList.resize(WindowSize);
    timeouts.resize(WindowSize);
    payloadList.resize(WindowSize);
}

void Node::initialize() {
    parametersInitialization();
}

void Node::IncrementSeqNum(int &seqNum) {
    seqNum++;
    seqNum %= (MAX_SEQ + 1);
}

void Node::IncrementWindowIndex(int &index) {
    index++;
    index %= WindowSize;
}

void Node::reFillTheWindow() {
    EV << "Start refill sender window" << std::endl;
    int i = 0;
    double totalTime = 0;
    while (currentWindowSize < WindowSize) {
        PacketInfo *packet = networkLayer.FromNetworkLayer();
        if (packet == nullptr)
            break;

        std::string NodeName = getFullName();
        double processingTime = simTime().dbl() + totalTime;

        EV << "At time [" << processingTime << "], Node[" << NodeName.back()
                  << "] , Introducing channel error with code =["
                  << packet->modification << packet->loss << packet->duplication
                  << packet->delay << "] ." << endl;

        Logger &logger = Logger::getInstance(OUTPUTFILEPATH);
        std::fstream &fout = logger.GetFileStream();
        if (fout.is_open()) {
            fout << "At time [" << processingTime << "], Node["
                    << NodeName.back()
                    << "] , Introducing channel error with code =["
                    << packet->modification << packet->loss
                    << packet->duplication << packet->delay << "] ." << endl;
        }

        Frame *frame = new Frame;
        frame->setSeqNum(seqNumber);

        ApplyByteStuffing(packet->message, frame);
        frame->setFrameType(2);
        ApplyCRC(frame);
        Frame *noErrorFrame = frame->dup();
        SeqList[endIndex] = noErrorFrame;

        std::vector<std::string> results = ApplyError(frame, packet->loss,
                packet->modification, packet->delay, packet->duplication);

        EV << "At time[" << simTime().dbl() + PT * (i + 1) << "], Node["
                  << NodeName.back() << "] [sent] frame with seq_num=["
                  << seqNumber << "] and payload=[" << frame->getPayload()
                  << "]" << " and trailer=[" << toBinary(frame->getTrailer())
                  << "]" << " , Modified [" << results[0] << "]" << " , Lost ["
                  << results[1] << "]" << ", Duplicate [" << results[2]
                  << "], Delay [" << (packet->delay ? ED : 0) << "]" << endl;

        if (fout.is_open()) {
            fout << "At time[" << simTime().dbl() + PT * (i + 1) << "], Node["
                    << NodeName.back() << "] [sent] frame with seq_num=["
                    << seqNumber << "] and payload=[" << frame->getPayload()
                    << "]" << " and trailer=[" << toBinary(frame->getTrailer())
                    << "]" << " , Modified [" << results[0] << "]"
                    << " , Lost [" << results[1] << "]" << ", Duplicate ["
                    << results[2] << "], Delay [" << (packet->delay ? ED : 0)
                    << "]" << endl;
        }

        if (!packet->loss) {
            sendDelayed(frame, PT * (i + 1) + TD + (packet->delay ? ED : 0),
                    "out");
            if (packet->duplication) {
                EV << "At time[" << simTime().dbl() + PT * (i + 1) + DD
                          << "], Node[" << NodeName.back()
                          << "] sent frame with seq_num=[" << seqNumber
                          << "] and payload=[" << frame->getPayload() << "]"
                          << " and trailer=[" << toBinary(frame->getTrailer())
                          << "]" << " , Modified [" << results[0] << "]"
                          << " , Lost [" << results[1] << "]" << ", Duplicate ["
                          << (int) (results[2][0] - '0') + 1 << "], Delay ["
                          << (packet->delay ? ED : 0) << "]" << endl;

                Logger &logger = Logger::getInstance(OUTPUTFILEPATH);
                std::fstream &fout = logger.GetFileStream();
                if (fout.is_open()) {
                    fout << "At time[" << simTime().dbl() + PT * (i + 1) + DD
                            << "], Node[" << NodeName.back()
                            << "] sent frame with seq_num=[" << seqNumber
                            << "] and payload=[" << frame->getPayload() << "]"
                            << " and trailer=[" << toBinary(frame->getTrailer())
                            << "]" << " , Modified [" << results[0] << "]"
                            << " , Lost [" << results[1] << "]"
                            << ", Duplicate ["
                            << (int) (results[2][0] - '0') + 1 << "], Delay ["
                            << (packet->delay ? ED : 0) << "]" << endl;
                }

                Frame *duplicateFrame = frame->dup();
                sendDelayed(duplicateFrame,
                        PT * (i + 1) + TD + DD + (packet->delay ? ED : 0),
                        "out");
            }
        }

        scheduleAt(simTime().dbl() + PT * (i + 1) + TO, noErrorFrame);
        timeouts[endIndex] = simTime().dbl() + PT * (i + 1) + TO;

        totalTime += PT;
        IncrementWindowIndex(endIndex);
        currentWindowSize++;
        IncrementSeqNum(seqNumber);
        i++;
    }
    EV << "End refill sender window" << std::endl;
}

void Node::MoveRecieverWindow(Frame *frame) {
    EV << "Start move rec window" << std::endl;
    if (payloadList[startIndex].second == false) {
        return;
    } else {
        while (payloadList[startIndex].second == true) {
            networkLayer.ToNetworkLayer(payloadList[startIndex].first.first,
                    payloadList[startIndex].first.second);
            payloadList[startIndex].second = false;
            IncrementWindowIndex(startIndex);
            IncrementWindowIndex(endIndex);
            IncrementSeqNum(seqNumber);
        }

        frame->setFrameType(1);
        frame->setACKNACKNumber(seqNumber);
        std::string NodeName = getFullName();

        srand(time(0));
        double randomNum = static_cast<double>(rand()) / RAND_MAX;

        EV << "At time[" << simTime().dbl() + PT << "], Node["
                  << NodeName.back() << "] Sending [ACK] with number ["
                  << seqNumber << "], loss[" << (randomNum >= LP ? "No" : "Yes")
                  << "] " << std::endl;

        Logger &logger = Logger::getInstance(OUTPUTFILEPATH);
        std::fstream &fout = logger.GetFileStream();
        if (fout.is_open()) {
            fout << "At time[" << simTime().dbl() + PT << "], Node["
                    << NodeName.back() << "] Sending [ACK] with number ["
                    << seqNumber << "], loss["
                    << (randomNum >= LP ? "No" : "Yes") << "] " << std::endl;
        }

        if (randomNum > LP) {
            sendDelayed(frame, TD + PT, "out");
        }
    }
    EV << "End move rec window" << std::endl;
}

void Node::handleRecieveData(Frame *frame) {
    EV << "Start Handle rec" << std::endl;
    if (((frame->getSeqNum() - seqNumber + (MAX_SEQ + 1)) % (MAX_SEQ + 1))
            >= WindowSize) {
        EV << "Frame seqNum not in receiver window" << std::endl;
    } else {

        if (checkCRC(frame)) {
            std::string payload = UnApplyByteStuffing(frame);
            payloadList[(startIndex
                    + ((frame->getSeqNum() - seqNumber + MAX_SEQ) % MAX_SEQ))
                    % WindowSize] = { { payload, frame->getSeqNum() }, true };
            if (frame->getSeqNum() != seqNumber) {
                frame->setFrameType(0);
                frame->setACKNACKNumber(seqNumber);
                std::string NodeName = getFullName();

                srand(time(0));
                double randomNum = static_cast<double>(rand()) / RAND_MAX;

                EV << "At time[" << simTime().dbl() + PT << "], Node["
                          << NodeName.back() << "] Sending [NACK] with number ["
                          << seqNumber << "], loss["
                          << (randomNum >= LP ? "No" : "Yes") << "] "
                          << std::endl;

                Logger &logger = Logger::getInstance(OUTPUTFILEPATH);
                std::fstream &fout = logger.GetFileStream();
                if (fout.is_open()) {
                    fout << "At time[" << simTime().dbl() + PT << "], Node["
                            << NodeName.back()
                            << "] Sending [NACK] with number [" << seqNumber
                            << "], loss[" << (randomNum >= LP ? "No" : "Yes")
                            << "] " << std::endl;
                }

                if (randomNum > LP) {
                    sendDelayed(frame, TD + PT, "out");
                }
            }
            MoveRecieverWindow(frame);
        } else {
            if (frame->getSeqNum() == seqNumber) {
                frame->setFrameType(0);
                frame->setACKNACKNumber(seqNumber);
                std::string NodeName = getFullName();

                srand(time(0));
                double randomNum = static_cast<double>(rand()) / RAND_MAX;

                EV << "At time[" << simTime().dbl() + PT << "], Node["
                          << NodeName.back() << "] Sending [NACK] with number ["
                          << seqNumber << "], loss["
                          << (randomNum >= LP ? "No" : "Yes") << "] "
                          << std::endl;

                Logger &logger = Logger::getInstance(OUTPUTFILEPATH);
                std::fstream &fout = logger.GetFileStream();
                if (fout.is_open()) {
                    fout << "At time[" << simTime().dbl() + PT << "], Node["
                            << NodeName.back()
                            << "] Sending [NACK] with number [" << seqNumber
                            << "], loss[" << (randomNum >= LP ? "No" : "Yes")
                            << "] " << std::endl;
                }

                if (randomNum >= LP) {
                    sendDelayed(frame, TD + PT, "out");
                }
            }
        }
    }
    EV << "End Handle rec" << std::endl;
}

void Node::handleACK(Frame *frame) {
    EV << "Start ACK" << std::endl;
    while (currentWindowSize
            && SeqList[startIndex]->getSeqNum() != frame->getACKNACKNumber()) {
        timeouts[startIndex] = 0;
        IncrementWindowIndex(startIndex);
        currentWindowSize--;
    }
    reFillTheWindow();
    EV << "END ACK" << std::endl;
}

void Node::handleNACK(Frame *frame) {
    EV << "Start NACK" << std::endl;
    int seqNum = frame->getACKNACKNumber();

    int index = 0;
    for (auto f : SeqList) {
        if (f->getSeqNum() == seqNum) {
            break;
        }
        index++;
    }

    frame = SeqList[index]->dup();

    sendDelayed(frame, PT + TD + 0.001, "out");

    Frame *noErrorFrame = frame->dup();
    scheduleAt(simTime().dbl() + PT + TO, noErrorFrame);
    timeouts[index] = simTime().dbl() + PT + TO;

    std::string NodeName = getFullName();
    EV << "At time[" << simTime().dbl() + PT << "], Node[" << NodeName.back()
              << "] sent frame with seq_num=[" << noErrorFrame->getSeqNum()
              << "] and payload=[" << frame->getPayload() << "]"
              << " and trailer=[" << toBinary(frame->getTrailer()) << "]"
              << " , Modified [" << -1 << "]" << " , Lost [" << "No" << "]"
              << ", Duplicate [" << 0 << "], Delay [" << 0 << "]" << endl;

    Logger &logger = Logger::getInstance(OUTPUTFILEPATH);
    std::fstream &fout = logger.GetFileStream();
    if (fout.is_open()) {
        fout << "At time[" << simTime().dbl() + PT << "], Node["
                << NodeName.back() << "] sent frame with seq_num=["
                << noErrorFrame->getSeqNum() << "] and payload=["
                << frame->getPayload() << "]" << " and trailer=["
                << toBinary(frame->getTrailer()) << "]" << " , Modified [" << -1
                << "]" << " , Lost [" << "No" << "]" << ", Duplicate [" << 0
                << "], Delay [" << 0 << "]" << endl;
    }

    EV << "End NACK" << std::endl;
}

void Node::handleTimeout(Frame *frame) {
    EV << "Start Timeout" << std::endl;
    int seqNum = frame->getSeqNum();

    if (WindowSize == 0) {
        return;
    }

    int index = 0;
    bool inWindow = false;
    for (auto f : SeqList) {
        if (f->getSeqNum() == seqNum) {
            inWindow = true;
            break;
        }
        index++;
    }

    if (inWindow && (simTime() == timeouts[index])) {
        sendDelayed(frame, PT + TD + 0.001, "out");

        Frame *noErrorFrame = frame->dup();
        scheduleAt(simTime().dbl() + PT + TO, noErrorFrame);
        timeouts[index] = simTime().dbl() + PT + TO;

        std::string NodeName = getFullName();
        EV << "Timeout event at [" << simTime().dbl() << "] at Node["
                  << NodeName.back() << "] for frame with seq_num=[" << seqNum
                  << "]" << endl;
        EV << "At time[" << simTime().dbl() + PT << "], Node["
                  << NodeName.back() << "] sent frame with seq_num=[" << seqNum
                  << "] and payload=[" << frame->getPayload() << "]"
                  << " and trailer=[" << toBinary(frame->getTrailer()) << "]"
                  << " , Modified [" << -1 << "]" << " , Lost [" << "No" << "]"
                  << ", Duplicate [" << 0 << "], Delay [" << 0 << "]" << endl;

        Logger &logger = Logger::getInstance(OUTPUTFILEPATH);
        std::fstream &fout = logger.GetFileStream();
        if (fout.is_open()) {
            fout << "Timeout event at [" << simTime().dbl() << "] at Node["
                    << NodeName.back() << "] for frame with seq_num=[" << seqNum
                    << "]" << endl;
            fout << "At time[" << simTime().dbl() + PT << "], Node["
                    << NodeName.back() << "] sent frame with seq_num=["
                    << seqNum << "] and payload=[" << frame->getPayload() << "]"
                    << " and trailer=[" << toBinary(frame->getTrailer()) << "]"
                    << " , Modified [" << -1 << "]" << " , Lost [" << "No"
                    << "]" << ", Duplicate [" << 0 << "], Delay [" << 0 << "]"
                    << endl;
        }

    }
    EV << "End Timeout" << std::endl;
}

void Node::handleMessage(cMessage *msg) {
    Frame *frame = dynamic_cast<Frame*>(msg);

    if (msg->isSelfMessage()) {
        handleTimeout(frame);
        return;
    }

    std::string msgType = frame->getName();
    if (msgType == "Coordinator") {
        std::string NodeName = getFullName();
        networkLayer.LoadPackets(NodeName);
        endIndex = 0;
    }
    switch (frame->getFrameType()) {
    case 0:
        handleNACK(frame);
        break;
    case 1:
        handleACK(frame);
        break;
    case 2:
        handleRecieveData(frame);
        break;
    default:
        reFillTheWindow();
        break;
    }
}
