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
        volatile double mean @unit(s) = 2.0s; //exponential distribution with the given mean (that is, with parameter lambda=1/mean). 
        volatile double powTime @unit(s) = 3.0s; //Proof of Work 
        
        //determines which TSA the node uses : IOTA or GIOTA or EIOTA
        string TSA = "EIOTA";
        
        //IOTA && GIOTA 
        double alpha = 5.0; //alpha in Pxy prob 
       
        //IOTA && GIOTA && EIOTA
        int W = 2500; //interval [W,2W]
       	int N = 100; //number of walkers

        //EIOTA
        double p1 = 0.1;
        double p2 = 0.65;
        double lowAlpha = 0.1;
    	double highAlpha = 5.0;
        
    gates:
        input NodeIn[];
        output NodeOut[];
}

network IOTA
{
    parameters:
        int transactionLimit = 100; //size of the Tangle 
        int NodeNumber = 10; //always > 0, number of nodes in the simulation
        
    submodules:
        Nodes[NodeNumber]: NodeModule;
        
    connections: //each module is connecting to his neighbors
        for i = 0 .. NodeNumber - 1, for j = 0 .. NodeNumber - 1, if i != j
        {
            Nodes[i].NodeOut++ --> {delay = 1ms;} --> Nodes[j].NodeIn++;
        }
}