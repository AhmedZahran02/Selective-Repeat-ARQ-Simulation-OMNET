# Selective Repeat ARQ Simulation (OMNeT++)

This repository contains a simulation of the Selective Repeat Automatic Repeat Request (ARQ) protocol, implemented using **OMNeT++**, a modular simulation framework for network modeling. The project is part of a course on **Computer Networks** at Cairo University.

## Overview
Selective Repeat ARQ is a reliable data link layer protocol used for error control in noisy communication channels. It allows multiple frames to be sent simultaneously while only retransmitting frames that are corrupted or lost during transmission.

This project simulates communication between two nodes over a noisy channel, implementing features such as:
- **Error handling:** detection and correction of transmission errors using Cyclic Redundancy Check (CRC).
- **Framing:** byte stuffing for framing of messages.
- **Selective retransmission:** only retransmitting erroneous or lost frames.
- **Event logging:** comprehensive logs for all communication events.
- **Simulation of channel conditions:** corruption, loss, duplication, and delay.

## Features
- **Selective Repeat ARQ Protocol:** Efficient error recovery with window-based retransmission.
- **Noisy Channel Simulation:** Models real-world conditions such as delay, corruption, duplication, and loss.
- **Error Detection:** CRC-based error checking.
- **Framing:** Byte stuffing for frame boundary detection.
- **Detailed Event Logging:** Tracks all events and interactions during the simulation.

## Installation
To run the simulation, follow these steps:

1. **Install OMNeT++**
   - Download and install OMNeT++ from the official website: [https://omnetpp.org](https://omnetpp.org)
   - Follow the installation instructions for your operating system.

2. **Clone this repository:**
   ```bash
   git clone https://github.com/AhmedZahran02/Selective-Repeat-ARQ-Simulation-OMNET
   cd Selective-Repeat-ARQ-Simulation-OMNET
   ```

3. **Build the project:**
   ```bash
   make
   ```

4. **Run the simulation:**
   ```bash
   ./run_simulation
   ```

## Usage
1. Configure the simulation parameters in the **omnetpp.ini** file.
2. Run the simulation and observe the output.
3. Check the event logs to see how errors are handled and frames are retransmitted.

## Directory Structure
```
Selective-Repeat-ARQ-Simulation-OMNET/
├── doc/                      # Documentation files
├── images/                   # Images for documentation
├── Makefile                  # Build configuration
├── omnetpp.ini               # Simulation configuration file
├── README.md                 # This file
├── src/                      # Source code for the simulation
│   ├── channel/              # Channel logic
│   ├── datalinklayer/        # Data link layer implementation
│   ├── networklayer/         # Network layer code (if implemented)
│   ├── packets/              # Packet definitions
│   ├── simulationlogic/      # Simulation management code
│   └── utils/                # Utility functions
└── test/                     # Test cases for the simulation
```

## Configuration Options
- Error rates for corruption, loss, and duplication.
- Transmission delays.
- Initial message assignment and transmission scheduling.

## Simulation Output Format
```
At time [0], Node[0] , Introducing channel error with code =[1010] .
At time[0.5], Node[0] [sent] frame with seq_num=[0] and payload=[$A nlower, sometimes $] and trailer=[11100010] , Modified [27] , Lost [No], Duplicate [1], Delay [0]
At time[0.5], Node[0] , Introducing channel error with code =[0000] .
At time[0.6], Node[0] [sent] frame with seq_num=[0] and payload=[$A nlower, sometimes $] and trailer=[11100010] , Modified [27] , Lost [No], Duplicate [2], Delay [0]
At time[1], Node[0] [sent] frame with seq_num=[1] and payload=[$known as a bloom or blossom in flowering plants$] and trailer=[11001000] , Modified [-1] , Lost [No], Duplicate [0], Delay [0]
At time [1], Node[0] , Introducing channel error with code =[0100] .
At time[1.5], Node[0] [sent] frame with seq_num=[2] and payload=[$is the reproductive/$ structure found  also called// angiosperms/$).$] and trailer=[11100001] , Modified [-1] , Lost [Yes], Duplicate [0], Delay [0]
Timeout event at [11.5] at Node[0] for frame with seq_num=[2]
Timeout event at [12] at Node[0] for frame with seq_num=[3]
```

## Contributing
Contributions are welcome! Feel free to open issues or submit pull requests.

## License
This project is licensed under the MIT License.

## Contact
Feel free to reach out to the repository owner or open an issue if you have any questions or suggestions. Happy coding! 🚀

