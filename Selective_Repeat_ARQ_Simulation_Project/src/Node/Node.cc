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

    processing = false;
    isInTimeout = false;
    isInNACK = false;

    no_nak = false;

    seqNumber = 0;
    startIndex = 0;
    endIndex = (MAX_SEQ + 1) / 2 - 1;
    currentWindowSize = 0;
    SeqList.resize(WindowSize);
    timeouts.resize(WindowSize);
    payloadList.resize(WindowSize);
    errors.resize(MAX_SEQ + 1);
    for (int i = 0; i <= MAX_SEQ; i++) {
        errors[i].resize(4);
    }
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
    if (!processing && currentWindowSize < WindowSize) {
        PacketInfo *packet = networkLayer.FromNetworkLayer();
        if (packet != nullptr) {
            std::string NodeName = getFullName();

            EV << "At time [" << simTime().dbl() << "], Node["
                      << NodeName.back()
                      << "] , Introducing channel error with code =["
                      << packet->modification << packet->loss
                      << packet->duplication << packet->delay << "] ." << endl;

            Logger &logger = Logger::getInstance(OUTPUTFILEPATH);
            std::fstream &fout = logger.GetFileStream();
            if (fout.is_open()) {
                fout << "At time [" << simTime().dbl() << "], Node["
                        << NodeName.back()
                        << "] , Introducing channel error with code =["
                        << packet->modification << packet->loss
                        << packet->duplication << packet->delay << "] ."
                        << endl;
            }

            Frame *frame = new Frame;
            frame->setSeqNum(seqNumber);

            ApplyByteStuffing(packet->message, frame);
            frame->setFrameType(2);
            ApplyCRC(frame);

            frame->setFrameType(3); // send to myself

            errors[seqNumber][0] = packet->loss;
            errors[seqNumber][1] = packet->modification;
            errors[seqNumber][2] = packet->delay;
            errors[seqNumber][3] = packet->duplication;

            processing = true;
            scheduleAt(simTime().dbl() + PT, frame);

            currentWindowSize++;
            IncrementSeqNum(seqNumber);
        }
    }
    EV << "End refill sender window" << std::endl;
}

void Node::MoveRecieverWindow(Frame *frame) {
    EV << "Start move rec window" << std::endl;
    if (payloadList[startIndex].second == false) {
        return;
    } else {
        no_nak = false;
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

        // ACK
        frame->setFrameType(frame->getFrameType() + 4);
        scheduleAt(simTime().dbl() + PT, frame);
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
                if(!no_nak){
                    no_nak = true;
                    frame->setFrameType(0);
                    frame->setACKNACKNumber(seqNumber);
                    // NACK
                    frame->setFrameType(frame->getFrameType() + 4);
                    scheduleAt(simTime().dbl() + PT, frame);
                }else{
                    // ACK
                    frame->setFrameType(1);
                    frame->setACKNACKNumber(seqNumber);
                    frame->setFrameType(frame->getFrameType() + 4);
                    scheduleAt(simTime().dbl() + PT, frame);
                }
            }
            MoveRecieverWindow(frame);
        } else {
            if (frame->getSeqNum() == seqNumber) {
                if (!no_nak) {
                    no_nak = true;
                    // NACK
                    frame->setFrameType(0);
                    frame->setFrameType(frame->getFrameType() + 4);
                    scheduleAt(simTime().dbl() + PT, frame);
                } else {
                    // ACK
                    frame->setFrameType(1);
                    frame->setACKNACKNumber(seqNumber);
                    frame->setFrameType(frame->getFrameType() + 4);
                    scheduleAt(simTime().dbl() + PT, frame);
                }

            }
        }
    }
    EV << "End Handle rec" << std::endl;
}

void Node::sendACKNACK(Frame *frame) {
    EV << "Start Sending ACKNACK" << std::endl;
    frame->setFrameType(frame->getFrameType() - 4);
    std::string NodeName = getFullName();
    srand(time(0));
    double randomNum = static_cast<double>(rand()) / RAND_MAX;

    if (frame->getFrameType() == 0) //NACK
            {
        EV << "At time[" << simTime().dbl() << "], Node[" << NodeName.back()
                  << "] Sending [NACK] with number ["
                  << frame->getACKNACKNumber() << "], loss["
                  << (randomNum >= LP ? "No" : "Yes") << "] " << std::endl;

        Logger &logger = Logger::getInstance(OUTPUTFILEPATH);
        std::fstream &fout = logger.GetFileStream();
        if (fout.is_open()) {
            fout << "At time[" << simTime().dbl() << "], Node["
                    << NodeName.back() << "] Sending [NACK] with number ["
                    << frame->getACKNACKNumber() << "], loss["
                    << (randomNum >= LP ? "No" : "Yes") << "] " << std::endl;
        }

        if (randomNum > LP) {
            sendDelayed(frame, TD, "out");
        }
    } else //ACK
    {
        EV << "At time[" << simTime().dbl() << "], Node[" << NodeName.back()
                  << "] Sending [ACK] with number ["
                  << frame->getACKNACKNumber() << "], loss["
                  << (randomNum >= LP ? "No" : "Yes") << "] " << std::endl;

        Logger &logger = Logger::getInstance(OUTPUTFILEPATH);
        std::fstream &fout = logger.GetFileStream();
        if (fout.is_open()) {
            fout << "At time[" << simTime().dbl() << "], Node["
                    << NodeName.back() << "] Sending [ACK] with number ["
                    << frame->getACKNACKNumber() << "], loss["
                    << (randomNum >= LP ? "No" : "Yes") << "] " << std::endl;
        }

        if (randomNum > LP) {
            sendDelayed(frame, TD + PT, "out");
        }
    }
    EV << "End Sending ACKNACK" << std::endl;
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

    frame->setFrameType(7);
    isInNACK = true;
    NACKEnding = simTime().dbl() + PT;
    scheduleAt(simTime().dbl() + PT, frame);

    timeouts[index] = simTime().dbl() + PT + TO;

    EV << "End NACK" << std::endl;
}

void Node::sendNACKData(Frame *frame) {
    isInNACK = false;
    frame->setFrameType(2);

    Frame *noErrorFrame = frame->dup();
    scheduleAt(simTime().dbl() + TO, noErrorFrame);

    sendDelayed(frame, TD + 0.001, "out");

    std::string NodeName = getFullName();
    EV << "At time[" << simTime().dbl() << "], Node[" << NodeName.back()
              << "] sent frame with seq_num=[" << noErrorFrame->getSeqNum()
              << "] and payload=[" << frame->getPayload() << "]"
              << " and trailer=[" << toBinary(frame->getTrailer()) << "]"
              << " , Modified [" << -1 << "]" << " , Lost [" << "No" << "]"
              << ", Duplicate [" << 0 << "], Delay [" << 0 << "]" << endl;

    Logger &logger = Logger::getInstance(OUTPUTFILEPATH);
    std::fstream &fout = logger.GetFileStream();
    if (fout.is_open()) {
        fout << "At time[" << simTime().dbl() << "], Node[" << NodeName.back()
                << "] sent frame with seq_num=[" << noErrorFrame->getSeqNum()
                << "] and payload=[" << frame->getPayload() << "]"
                << " and trailer=[" << toBinary(frame->getTrailer()) << "]"
                << " , Modified [" << -1 << "]" << " , Lost [" << "No" << "]"
                << ", Duplicate [" << 0 << "], Delay [" << 0 << "]" << endl;
    }
}

void Node::handleTimeout(Frame *frame) {
    EV << "Start Timeout" << std::endl;

    if (WindowSize == 0) {
        return;
    }

    int index = 0;
    bool inWindow = false;
    for (auto f : SeqList) {
        if (f && f->getSeqNum() == frame->getSeqNum()) {
            inWindow = true;
            break;
        }
        index++;
    }

    if (inWindow && (simTime() == timeouts[index])) {
        frame->setFrameType(6); // timeout
        std::string NodeName = getFullName();
        EV << "Timeout event at [" << simTime().dbl() << "] at Node["
                  << NodeName.back() << "] for frame with seq_num=["
                  << frame->getSeqNum() << "]" << endl;
        Logger &logger = Logger::getInstance(OUTPUTFILEPATH);
        std::fstream &fout = logger.GetFileStream();
        if (fout.is_open()) {
            fout << "Timeout event at [" << simTime().dbl() << "] at Node["
                    << NodeName.back() << "] for frame with seq_num=["
                    << frame->getSeqNum() << "]" << endl;
        }
        isInTimeout = true;
        TimeoutEnding = simTime().dbl() + PT;
        scheduleAt(simTime().dbl() + PT, frame);
        timeouts[index] = simTime().dbl() + PT + TO;
    }
    EV << "End Timeout" << std::endl;
}

void Node::sendTimeout(Frame *frame) {
    isInTimeout = false;
    frame->setFrameType(2);

    sendDelayed(frame, TD + 0.001, "out");

    Frame *noErrorFrame = frame->dup();
    scheduleAt(simTime().dbl() + TO, noErrorFrame);

    std::string NodeName = getFullName();

    EV << "At time[" << simTime().dbl() << "], Node[" << NodeName.back()
              << "] sent frame with seq_num=[" << frame->getSeqNum()
              << "] and payload=[" << frame->getPayload() << "]"
              << " and trailer=[" << toBinary(frame->getTrailer()) << "]"
              << " , Modified [" << -1 << "]" << " , Lost [" << "No" << "]"
              << ", Duplicate [" << 0 << "], Delay [" << 0 << "]" << endl;

    Logger &logger = Logger::getInstance(OUTPUTFILEPATH);
    std::fstream &fout = logger.GetFileStream();
    if (fout.is_open()) {
        fout << "At time[" << simTime().dbl() << "], Node[" << NodeName.back()
                << "] sent frame with seq_num=[" << frame->getSeqNum()
                << "] and payload=[" << frame->getPayload() << "]"
                << " and trailer=[" << toBinary(frame->getTrailer()) << "]"
                << " , Modified [" << -1 << "]" << " , Lost [" << "No" << "]"
                << ", Duplicate [" << 0 << "], Delay [" << 0 << "]" << endl;
    }
}

void Node::sendFrame(Frame *frame) {
    if (isInTimeout) {
        scheduleAt(TimeoutEnding + PT, frame);
        return;
    }
    if (isInNACK) {
        scheduleAt(NACKEnding + PT, frame);
        return;
    }
    processing = false;
    EV << "Start Sending DATA" << std::endl;
    frame->setFrameType(2);
    Frame *noErrorFrame = frame->dup();
    SeqList[endIndex] = noErrorFrame;

    std::string NodeName = getFullName();
    Logger &logger = Logger::getInstance(OUTPUTFILEPATH);
    std::fstream &fout = logger.GetFileStream();

    bool loss, modification, delay, duplication;
    loss = errors[frame->getSeqNum()][0];
    modification = errors[frame->getSeqNum()][1];
    delay = errors[frame->getSeqNum()][2];
    duplication = errors[frame->getSeqNum()][3];

    std::vector<std::string> results = ApplyError(frame, loss, modification,
            delay, duplication);

    EV << "At time[" << simTime().dbl() << "], Node[" << NodeName.back()
              << "] [sent] frame with seq_num=[" << frame->getSeqNum()
              << "] and payload=[" << frame->getPayload() << "]"
              << " and trailer=[" << toBinary(frame->getTrailer()) << "]"
              << " , Modified [" << results[0] << "]" << " , Lost ["
              << results[1] << "]" << ", Duplicate [" << results[2]
              << "], Delay [" << (delay ? ED : 0) << "]" << endl;

    if (fout.is_open()) {
        fout << "At time[" << simTime().dbl() << "], Node[" << NodeName.back()
                << "] [sent] frame with seq_num=[" << frame->getSeqNum()
                << "] and payload=[" << frame->getPayload() << "]"
                << " and trailer=[" << toBinary(frame->getTrailer()) << "]"
                << " , Modified [" << results[0] << "]" << " , Lost ["
                << results[1] << "]" << ", Duplicate [" << results[2]
                << "], Delay [" << (delay ? ED : 0) << "]" << endl;
    }

    if (duplication) {
        EV << "At time[" << simTime().dbl() + DD << "], Node["
                  << NodeName.back() << "] sent frame with seq_num=["
                  << frame->getSeqNum() << "] and payload=["
                  << frame->getPayload() << "]" << " and trailer=["
                  << toBinary(frame->getTrailer()) << "]" << " , Modified ["
                  << results[0] << "]" << " , Lost [" << results[1] << "]"
                  << ", Duplicate [" << (int) (results[2][0] - '0') + 1
                  << "], Delay [" << (delay ? ED : 0) << "]" << endl;

        Logger &logger = Logger::getInstance(OUTPUTFILEPATH);
        std::fstream &fout = logger.GetFileStream();
        if (fout.is_open()) {
            fout << "At time[" << simTime().dbl() + DD << "], Node["
                    << NodeName.back() << "] sent frame with seq_num=["
                    << frame->getSeqNum() << "] and payload=["
                    << frame->getPayload() << "]" << " and trailer=["
                    << toBinary(frame->getTrailer()) << "]" << " , Modified ["
                    << results[0] << "]" << " , Lost [" << results[1] << "]"
                    << ", Duplicate [" << (int) (results[2][0] - '0') + 1
                    << "], Delay [" << (delay ? ED : 0) << "]" << endl;
        }
    }

    if (!loss) {
        sendDelayed(frame, TD + (delay ? ED : 0), "out");
        if (duplication) {
            Frame *duplicateFrame = frame->dup();
            sendDelayed(duplicateFrame, TD + DD + (delay ? ED : 0), "out");
        }
    }

    scheduleAt(simTime().dbl() + TO, noErrorFrame);
    timeouts[endIndex] = simTime().dbl() + TO;
    IncrementWindowIndex(endIndex);
    reFillTheWindow();
    EV << "End Sending DATA" << std::endl;
}

void Node::handleMessage(cMessage *msg) {
    Frame *frame = dynamic_cast<Frame*>(msg);

    if (msg->isSelfMessage() && frame->getFrameType() == 2) {
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
    case 3:
        sendFrame(frame);
        break;
    case 4:
        sendACKNACK(frame);
        break;
    case 5:
        sendACKNACK(frame);
        break;
    case 6:
        sendTimeout(frame);
        break;
    case 7:
        sendNACKData(frame);
    default:
        reFillTheWindow();
        break;
    }
}
