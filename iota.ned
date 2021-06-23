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

//NodeModule : user issuing transactions 
simple NodeModule
{
    parameters:
        volatile double trgenRate @unit(s) = 3.0s; //how often a node will try to issue a transaction
        volatile double prob = 0.3; //the probability to issue a transaction (must be between 0 and 100)
        volatile double powTime @unit(s) = 10.0s; //Proof of Work 
        
        //determines which TSA the node uses : IOTA - GIOTA - EIOTA
        string TSA = "IOTA";
        
        //IOTA && GIOTA && EIOTA
        double alpha = 5.0; //alpha in Pxy prob 
        int W = 250; //interval [W,2W]
        int N = 100; //number of walkers

        //EIOTA
        double p1 = 0.1;
        double p2 = 0.65;
        
    gates:
        input NodeIn[];
        output NodeOut[];
}

network IOTA
{
    parameters:
        int transactionLimit = 100; //size of the Tangle 
        int NodeNumber = 2; //always > 0, number of sites in the simulation
        
    submodules:
        Nodes[NodeNumber]: NodeModule;
        
    connections: //each module is connecting to his neighbors
        for i = 0 .. NodeNumber - 1, for j = 0 .. NodeNumber - 1, if i != j
        {
            Nodes[i].NodeOut++ --> {delay = 1ms;} --> Nodes[j].NodeIn++;
        }
}
