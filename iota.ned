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
        //here we take the transaction rate of VISA which is 1700 tx per sec, hence, each tx is spaced by 0.000588s
        volatile double mean @unit(s) = 0.000588s; //exponential distribution with the given mean (that is, with parameter lambda=1/mean). 
        volatile double powTime @unit(s) = 0.0002s; //Proof of Work 
        
        //determines which TSA the node uses : IOTA or GIOTA or EIOTA
        string TSA = "EIOTA";
        
        //IOTA && GIOTA 
        double alpha = 5.0; //alpha in Pxy prob 
       
        //IOTA && GIOTA && EIOTA
        int W = 250; //interval [W,2W]
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

// A generated network with complete graph topology.
network FullGraph
{
    parameters:
        //always > 0, number of nodes in the simulation
        int NodeNumber = default(5); 
        
        //how much transactions each node will issue
        int transactionLimit = 100; 
        
        //random delay
    	bool ifRandDelay = false;
    	
    	//parameters used by the uniform distribution during NodeModule initialization for setting the delay in SECONDS !
        double minDelay = 50.0; 
        double maxDelay = 100.0;
        
        //confidence lvl (must be between 0.0 - 1.0)
        double Confidencelvl = 0.8;
    
	types:
        channel Channel extends ned.DelayChannel
	    {
	        delay = 0.01ms;
	    }
      
    submodules:
        Nodes[NodeNumber]: NodeModule;
        
    connections: //each module is connecting to his neighbors
        for i = 0 .. NodeNumber - 1, for j = 0 .. NodeNumber - 1, if i != j
        {
            Nodes[i].NodeOut++ --> Channel --> Nodes[j].NodeIn++;
        }
}

// A generated network with grid topology.
network Grid
{
    parameters:
        //dimensions of the Grid
        int columns = default(5);
        int rows = default(5);
        
        //how much transactions each node will issue
        int transactionLimit = 100; 
        
        //random delay
    	bool ifRandDelay = false;
    	
    	//parameters used by the uniform distribution during NodeModule initialization for setting the delay in SECONDS !
        double minDelay = 50.0; 
        double maxDelay = 100.0;
        
        //confidence lvl (must be between 0.0 - 1.0)
        double Confidencelvl = 0.8;
        
	types:
        channel Channel extends ned.DelayChannel
	    {
	        delay = 1ms;
	    }
      
    submodules:
        Nodes[columns*rows]: NodeModule;
    
    connections allowunconnected:
        for i = 0 .. rows - 1, for j = 0 .. columns - 1 
        {
            Nodes[i*columns+j].NodeOut++ --> Channel --> Nodes[(i+1)*columns+j].NodeIn++ if i != rows - 1;
            Nodes[i*columns+j].NodeOut++ --> Channel --> Nodes[i*columns+j+1].NodeIn++ if j != columns - 1;
            
            Nodes[(i+1)*columns+j].NodeOut++ --> Channel --> Nodes[i*columns+j].NodeIn++ if i != rows - 1;
            Nodes[i*columns+j+1].NodeOut++ --> Channel --> Nodes[i*columns+j].NodeIn++ if j != columns - 1;
        }
}

// A generated network with torus topology.
network Torus
{
    parameters:
        //dimensions of the Torus
        int columns = default(5);
        int rows = default(5);
        
        //how much transactions each node will issue
        int transactionLimit = 100; 
        
        //random delay
    	bool ifRandDelay = false;
    	
    	//parameters used by the uniform distribution during NodeModule initialization for setting the delay in SECONDS !
        double minDelay = 50.0; 
        double maxDelay = 100.0;
        
        //confidence lvl (must be between 0.0 - 1.0)
        double Confidencelvl = 0.8;
        
    types:
        channel Channel extends ned.DelayChannel
	    {
	        delay = 1ms;
	    }
      
    submodules:
        Nodes[columns*rows]: NodeModule;
        
    connections:
        for i=0..rows-1, for j=0..columns-1
        {
            Nodes[i*columns+j].NodeOut++ --> Channel --> Nodes[((i+1)%rows)*columns+j].NodeIn++ if i != rows - 1;
            Nodes[i*columns+j].NodeOut++ --> Channel --> Nodes[(i*columns+(j+1))%columns].NodeIn++ if j != columns - 1;
            
            Nodes[((i+1)%rows)*columns+j].NodeOut++ --> Channel --> Nodes[i*columns+j].NodeIn++ if i != rows - 1;
            Nodes[(i*columns+(j+1))%columns].NodeOut++ --> Channel --> Nodes[i*columns+j].NodeIn++ if j != columns - 1;
        }
}

// A generated network with expander topology (uses csv file generated by python script)
network Expander
{
    parameters: 
        //always > 0, number of nodes in the simulation
        int NodeNumber = default(10); 
        
        //how much transactions each node will issue
        int transactionLimit = 100;
        
        //Initial delay
        double delay = 0.001; 
        
        //random delay
    	bool ifRandDelay = true;
    	
    	//parameters used by the uniform distribution during NodeModule initialization for setting the delay in SECONDS !
        double minDelay = 50.0; 
        double maxDelay = 100.0;
        
        //confidence lvl (must be between 0.0 - 1.0)
        double Confidencelvl = 0.8;
        
    submodules:
        Nodes[NodeNumber]: NodeModule;
}

// A generated network with watts-strogatz topology (uses csv file generated by python script)
network WattsStrogatz
{
    parameters: 
        //always > 0, number of nodes in the simulation
        int NodeNumber = default(10); 
        
        //how much transactions each node will issue
        int transactionLimit = 100;
        
        //Initial delay
        double delay = 0.001;
        
        //random delay
    	bool ifRandDelay = false;
    	
    	//parameters used by the uniform distribution during NodeModule initialization for setting the delay in SECONDS !
        double minDelay = 50.0; 
        double maxDelay = 100.0;
        
        //confidence lvl (must be between 0.0 - 1.0)
        double Confidencelvl = 0.8;
        
    submodules:
        Nodes[NodeNumber]: NodeModule;
}
