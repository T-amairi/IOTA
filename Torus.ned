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
// A generated network with torus topology.
//

network Torus
{
    parameters:
        //dimensions of the Grid
        int columns = default(5);
        int rows = default(5);
        
        //how much transactions each node will issue
        int transactionLimit = 100; 
        
        //parameters used by the uniform distribution during NodeModule initialization for setting the delay in SECONDS !
        double minDelay = 50.0; 
        double maxDelay = 100.0;
      
    submodules:
        Nodes[columns*rows]: NodeModule;
        
    connections:
        for i=0..rows-1, for j=0..columns-1
        {
            Nodes[i*columns+j].NodeOut++ --> {delay = 1 ms;} --> Nodes[((i+1)%rows)*columns+j].NodeIn++ if i != rows - 1;
            Nodes[i*columns+j].NodeOut++ --> {delay = 1 ms;} --> Nodes[(i*columns+(j+1))%columns].NodeIn++ if j != columns - 1;
            
            Nodes[((i+1)%rows)*columns+j].NodeOut++ --> {delay = 1 ms;} --> Nodes[i*columns+j].NodeIn++ if i != rows - 1;
            Nodes[(i*columns+(j+1))%columns].NodeOut++ --> {delay = 1 ms;} --> Nodes[i*columns+j].NodeIn++ if j != columns - 1;
        }
}
