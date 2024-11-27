package selectiverepeatarqsimulationproject;

import ned.IdealChannel;
import selectiverepeatarqsimulationproject.Coordinator.Coordinator;
import selectiverepeatarqsimulationproject.Node.Node;


@license(LGPL);

network Network
{
    @display("bgb=339.69333,255.98666");
    submodules:
        node0: Node {
            @display("p=70.08,98.30666");
        }
        node1: Node {
            @display("p=256.96,98.30666");
        }
        coordinator: Coordinator {
            @display("p=163.52,175.2");
        }
    connections:
        node0.out --> IdealChannel --> node1.in;
        node1.out --> IdealChannel --> node0.in;
        coordinator.node1 --> IdealChannel --> node0.coordinatorIn;
        coordinator.node2 --> IdealChannel --> node1.coordinatorIn;
}

