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
        double rateMean @unit(s) = 0.5s; //exponential distribution with the given mean  (!= 0 because lambda = 1 / rateMean)
        double powTime @unit(s) = 0.00s; //Proof of Work 
        int transactionLimit = 10; //how much transactions the node will issue
        string TSA = "IOTA"; //determines which TSA the node uses : IOTA or GIOTA or EIOTA
		
		//******* IOTA & GIOTA & EIOTA *******//
        //for the interval [W,2W] and have to be in [0,1]:  
        //e.g if WProp = 0.5 -> W = 0.5 * myTangle.size()
        double WProp = 0.25;
        //to limit the value of W based on the final size of the Tangle:
        //e.g if WPercentageThreshold = 0.10 -> W <= 0.10 * myTangle.finalSize()
        double WPercentageThreshold = 0.10;
        int N = 50; //number of walkers
        
        //******* IOTA & GIOTA *******//
        double alpha = 0.08; //alpha in Pxy prob 

        //******* GIOTA *******//
        //the maximum number of steps to be performed when computing the confidence
        int confDistance = 20;

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
        //determine the computing power of the malicious node according to rateMean:
        //e.g if PropComputingPower = 0.5, the node can send twice as many transactions
        double propComputingPower = 0.02;
        //the closer to 1, the higher the proportion of transactions in the chain is and vice versa
        double propChainTips = 0.0; 

        //******* SPlITTING ATTACK *******// 
        bool SPattack = false; //to initiate a Splitting Attack
        double sizeBrancheProp = 10; //sizes of the branches at initialization
        //by how much is multiplied rateMean, this allows to obtain the power of the malicious node: 
        //e.g if PropRateMB = 0.5, the attacker is twice as fast as an honest node
        double propRateMB = 0.08;
        
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
        double delay = 0.2; //the delay in SECONDS !
        bool ifRandDelay = false; //to set up a random delay
        //parameter used by the uniform distribution during initialization for setting the delay in SECONDS !
        double minDelay = 50.0;
        double maxDelay = 100.0;

    submodules:
        Configurator: ConfiguratorModule;
        Malicious[nbMaliciousNode]: MaliciousModule;
        Honest[nbHonestNode]: HonestModule;     
}
