//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

import NodeModule;

channel Link extends ned.DatarateChannel {
    parameters:
        int cost = default(0);
}

//
// Network generated from /IOTA/topologies/exp_CSV/expander0.csv 
//
network Expander {
	parameters:
		//how much transactions each node will issue
        int transactionLimit = 100; 
       
        //parameters used by the uniform distribution during NodeModule initialization for setting the delay in SECONDS !
        double minDelay = 50.0; 
        double maxDelay = 100.0;
	
    submodules:
        Node0: NodeModule;
        Node1: NodeModule;
        Node2: NodeModule;
        Node3: NodeModule;
        Node4: NodeModule;
        Node5: NodeModule;
        Node6: NodeModule;
        Node7: NodeModule;
        Node8: NodeModule;
        Node9: NodeModule;
    connections:
        Node0.NodeOut++ --> {delay = 1 ms;} --> Node9.NodeIn++;
        Node9.NodeOut++ --> {delay = 1 ms;} --> Node0.NodeIn++;
        Node0.NodeOut++ --> {delay = 1 ms;} --> Node1.NodeIn++;
        Node1.NodeOut++ --> {delay = 1 ms;} --> Node0.NodeIn++;
        Node9.NodeOut++ --> {delay = 1 ms;} --> Node8.NodeIn++;
        Node8.NodeOut++ --> {delay = 1 ms;} --> Node9.NodeIn++;
        Node9.NodeOut++ --> {delay = 1 ms;} --> Node1.NodeIn++;
        Node1.NodeOut++ --> {delay = 1 ms;} --> Node9.NodeIn++;
        Node1.NodeOut++ --> {delay = 1 ms;} --> Node2.NodeIn++;
        Node2.NodeOut++ --> {delay = 1 ms;} --> Node1.NodeIn++;
        Node1.NodeOut++ --> {delay = 1 ms;} --> Node3.NodeIn++;
        Node3.NodeOut++ --> {delay = 1 ms;} --> Node1.NodeIn++;
        Node1.NodeOut++ --> {delay = 1 ms;} --> Node7.NodeIn++;
        Node7.NodeOut++ --> {delay = 1 ms;} --> Node1.NodeIn++;
        Node2.NodeOut++ --> {delay = 1 ms;} --> Node3.NodeIn++;
        Node3.NodeOut++ --> {delay = 1 ms;} --> Node2.NodeIn++;
        Node2.NodeOut++ --> {delay = 1 ms;} --> Node6.NodeIn++;
        Node6.NodeOut++ --> {delay = 1 ms;} --> Node2.NodeIn++;
        Node3.NodeOut++ --> {delay = 1 ms;} --> Node4.NodeIn++;
        Node4.NodeOut++ --> {delay = 1 ms;} --> Node3.NodeIn++;
        Node6.NodeOut++ --> {delay = 1 ms;} --> Node4.NodeIn++;
        Node4.NodeOut++ --> {delay = 1 ms;} --> Node6.NodeIn++;
        Node6.NodeOut++ --> {delay = 1 ms;} --> Node5.NodeIn++;
        Node5.NodeOut++ --> {delay = 1 ms;} --> Node6.NodeIn++;
        Node6.NodeOut++ --> {delay = 1 ms;} --> Node7.NodeIn++;
        Node7.NodeOut++ --> {delay = 1 ms;} --> Node6.NodeIn++;
        Node6.NodeOut++ --> {delay = 1 ms;} --> Node8.NodeIn++;
        Node8.NodeOut++ --> {delay = 1 ms;} --> Node6.NodeIn++;
        Node4.NodeOut++ --> {delay = 1 ms;} --> Node5.NodeIn++;
        Node5.NodeOut++ --> {delay = 1 ms;} --> Node4.NodeIn++;
        Node7.NodeOut++ --> {delay = 1 ms;} --> Node8.NodeIn++;
        Node8.NodeOut++ --> {delay = 1 ms;} --> Node7.NodeIn++;
}
