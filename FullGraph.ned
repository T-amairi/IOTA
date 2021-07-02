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

// Created: 2021-6-2 for project IOTA

import NodeModule;

//
// A generated network with complete graph topology.
//

network FullGraph
{
    parameters:
        //always > 0, number of nodes in the simulation
        int NodeNumber = default(5); 
        
        //how much transactions each node will issue
        int transactionLimit = 100; 
       
        //parameters used by the uniform distribution during NodeModule initialization for setting the delay in SECONDS !
        double minDelay = 50.0; 
        double maxDelay = 100.0;
      
    submodules:
        Nodes[NodeNumber]: NodeModule;
        
    connections: //each module is connecting to his neighbors
        for i = 0 .. NodeNumber - 1, for j = 0 .. NodeNumber - 1, if i != j
        {
            Nodes[i].NodeOut++ --> {delay = 1 ms;} --> Nodes[j].NodeIn++;
        }
}