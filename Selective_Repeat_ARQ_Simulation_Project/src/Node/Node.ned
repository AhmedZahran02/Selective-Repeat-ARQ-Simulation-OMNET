package selectiverepeatarqsimulationproject.Node;

simple Node
{
    parameters:
        int WindowSize = default(4);
        int MAX_SEQ = default(7);
        double TO = default(10.0);
        double PT = default(0.5);
        double TD = default(1.0);
        double ED = default(4.0);
        double DD = default(0.1);
        double LP = default(0.0);
	gates:
	    input coordinatorIn;
	    input in;
        output out;
}