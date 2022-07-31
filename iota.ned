//ConfiguratorModule : non-networked module setting up the topology
simple ConfiguratorModule
{
    @display("i=block/cogwheel");
}

//HonestModule : honest user issuing transactions  
simple HonestModule 
{
    parameters:
    	@display("i=,blue");
    	
        //******* PARAMETERS *******//
        double rateMean @unit(s) = 0.5s; //exponential distribution with the given mean  (!= 0)
        double powTime @unit(s) = 0.00s; //Proof of Work 
        int transactionLimit = 100; //how much transactions the node will issue
        string TSA = "IOTA"; //determines which TSA the node uses : IOTA or GIOTA or EIOTA
		
		//******* IOTA & GIOTA & EIOTA *******//
        double WProp = 0.25; //for the interval [W,2W] and have to be in [0,1] :  e.g if WProp = 0.5 => W = 0.5 * myTangle.size()
        int N = 50; //number of walkers
        
        //******* IOTA & GIOTA *******//
        double alpha = 0.8; //alpha in Pxy prob 

        //******* GIOTA *******//
        int confDistance = 20; //the maximum number of steps to be performed when computing the confidence

        //******* EIOTA *******//
        double p1 = 0.1;
        double p2 = 0.65;
        double lowAlpha = 0.08;
        double highAlpha = 2.5;
        
        //******* LOG *******//
        bool exportTangle = false;
        bool exportTipsNumber = true;
        bool wipeLogTipsNumber = false;
         
    gates:
        input in[];
        output out[];      
}

//MaliciousModule : a malicious node launching attacks
simple MaliciousModule extends HonestModule
{
    parameters:
        @class(MaliciousModule);
    	@display("i=,red");
    	
        //when initiate an attack 
        double attackStage = 0.25; //have to be in [0,1] : based on the size of the Tangle

        //******* PARASITE CHAIN ATTACK *******//
        bool PCattack = false; //to initiate a Parasite Chain Attack
        double propComputingPower = 0.02; //determine the computing power of the malicious node according to rateMean (e.g if PropComputingPower = 0.5, the node can send twice as many transactions)
        double propChainTips = 0.0; //the closer to 1, the higher the proportion of transactions in the chain is and vice versa  

        //******* SPlITTING ATTACK *******// 
        bool SPattack = false; //to initiate a Splitting Attack
        double sizeBrancheProp = 10; //sizes of the branches at ini 
        double propRateMB = 0.08; //by how much is multiplied rateMean, this allows to obtain the power of the malicious node (e.g if PropRateMB = 0.5, the attacker is twice as fast as an honest node).
        
        //******* LOG *******//
        bool exportConflictTx = false;
        bool exportDiffTxChain = false;
        bool wipeLogDiffTxChain = false;
        bool exportBranchSize = false;
        bool wipeLogBranchSize = false;
}

//A generated network based on the python script "GenTopo.py" in the topologies folder
network Network
{
    parameters:
        //name of the used topology : FullGraph or Grid or Torus or WattsStrogatz
        string topology = "WattsStrogatz"; //this parameter is case sensitive, be careful !

		//******* NODES *******//
		int nbMaliciousNode = 0; //number of malicious nodes in the simulation
        int nbHonestNode = 100; //number of honest nodes in the simulation
        
		//******* DELAY *******// 
        double delay = 0.25; //the delay in SECONDS !
        bool ifRandDelay = false; //to set up a random delay
        double minDelay = 50.0; //parameter used by the uniform distribution during initialization for setting the delay in SECONDS !
        double maxDelay = 100.0; //parameter used by the uniform distribution during initialization for setting the delay in SECONDS !

    submodules:
        Configurator: ConfiguratorModule;
        Malicious[nbMaliciousNode]: MaliciousModule;
        Honest[nbHonestNode]: HonestModule;     
}
