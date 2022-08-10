#include "ConfiguratorModule.h"

std::vector<int> ConfiguratorModule::split(const std::string& line) const
{
    std::vector<int> tokens;
    std::string token;
    std::istringstream tokenStream(line);

    while(std::getline(tokenStream, token, ','))
    {
        tokens.push_back(stoi(token));
    }

    return tokens;
}

std::map<int,std::vector<int>> ConfiguratorModule::getEdgeList() const
{
    std::string currentTopo = getParentModule()->par("topology");
    std::string path = "./topologies/CSV/" + currentTopo + ".csv";
    std::fstream file;
    file.open(path,std::ios::in);

    if(!file.is_open()) throw std::runtime_error("Could not open CSV file");

    std::string line;
    std::map<int,std::vector<int>> myTopo;

    while(getline(file,line))
    {
        auto edgeList = split(line);
        myTopo[edgeList[0]] = std::vector<int>(edgeList.begin() + 1, edgeList.begin() + edgeList.size());
    }

    file.close();

    return myTopo;
}

void ConfiguratorModule::connectModules(cModule* moduleOut, cModule* moduleIn, randomNumberGenerator& myRNG)
{
    double delay = 0.0;

    if(getParentModule()->par("ifRandDelay"))
    {
       double minDelay = getParentModule()->par("minDelay");
       double maxDelay = getParentModule()->par("maxDelay");
       delay = myRNG.doubleUniform(minDelay,maxDelay);
    }

    else
    {
       delay = getParentModule()->par("delay");
    }

    auto channel = cDelayChannel::create("Channel");
    channel->setDelay(delay);

    moduleOut->setGateSize("out",moduleOut->gateSize("out") + 1);
    moduleIn->setGateSize("in",moduleIn->gateSize("in") + 1);

    auto gOut = moduleOut->gate("out",moduleOut->gateSize("out") - 1);
    auto gIn = moduleIn->gate("in",moduleIn->gateSize("in") - 1);

    gOut->connectTo(gIn,channel);
    gOut->getChannel()->callInitialize();
}

std::vector<cModule*> ConfiguratorModule::getModules() const
{
    std::vector<cModule*> myModules;
    int nbMaliciousNode = getParentModule()->par("nbMaliciousNode");
    int nbHonestNode = getParentModule()->par("nbHonestNode");

    for(int i = 0; i < nbMaliciousNode; i++)
    {
        myModules.push_back(getParentModule()->getSubmodule("Malicious",i));
    }

    for(int i = 0; i < nbHonestNode; i++)
    {
        myModules.push_back(getParentModule()->getSubmodule("Honest",i));
    }

    return myModules;
}

void ConfiguratorModule::checkError(int nodeID, int totalModules) const
{
    if(nodeID < 0 || nodeID >= totalModules)
    {
        throw std::runtime_error("Module not found while setting connections : check the number of nodes set in the python script and in the .ned file, they have to be identical !");
    }
}

void ConfiguratorModule::logConfig() const
{
    std::cout << std::boolalpha;
    std::cout << "\n~~~~~~~~~~~~ OMNeT++ IOTA SIMULATION CONFIGURATION ~~~~~~~~~~~~\n";

    auto network = getParentModule();
    auto honestModule = network->getSubmodule("Honest",0);
    auto maliciousModule = network->getSubmodule("Malicious",0);

    std::cout << "\n-------------- Network settings --------------\n"
         << "* Selected topology: " << std::string(network->par("topology")) << "\n"
         << "* Number of malicious nodes: " << int(network->par("nbMaliciousNode")) << "\n"
         << "* Number of honest nodes: " << int(network->par("nbHonestNode")) << "\n";

    bool ifRandDelay = network->par("ifRandDelay");
    std::cout << "* To set up a random delay based on an interval: " << ifRandDelay << "\n";

    if(ifRandDelay)
    {
        std::cout << "\t- Minimum delay: " << double(network->par("minDelay")) << " seconds\n";
        std::cout << "\t- Maximum delay: " << double(network->par("maxDelay")) << " seconds\n";
    }

    else
    {
        std::cout << "\t- Fixed delay: " << double(network->par("delay")) << " seconds\n";
    }

    if(honestModule)
    {
        std::cout << "\n------------ Honest node settings ------------\n"
             << "* The exponential distribution mean of the issuing rate: " << double(honestModule->par("rateMean")) << " per seconds\n"
             << "* Proof of work time: " << double(honestModule->par("powTime")) << " seconds\n"
             << "* Transaction limit: " << int(honestModule->par("transactionLimit")) << "\n";

        std::string TSA = honestModule->par("TSA");
        std::cout << "* Tips selection algorithm: " << TSA << "\n"
             << "\t- WProp: " << double(honestModule->par("WProp")) << "\n"
             << "\t- WPercentageThreshold: " << double(honestModule->par("WPercentageThreshold")) << "\n"
             << "\t- N: " << int(honestModule->par("N")) << "\n";

        if(TSA == "IOTA" || TSA == "GIOTA")
        {
            std::cout << "\t- Alpha: " << double(honestModule->par("alpha")) << "\n";

            if(TSA == "GIOTA")
            {
                std::cout << "\t- confDistance: " << int(honestModule->par("confDistance")) << "\n";
            }
        }

        else
        {
            std::cout << "\t- p1: " << double(honestModule->par("p1")) << "\n"
                 << "\t- p2: " << double(honestModule->par("p2")) << "\n"
                 << "\t- Low alpha: " << double(honestModule->par("lowAlpha")) << "\n"
                 << "\t- High alpha: " << double(honestModule->par("highAlpha")) << "\n";
        }

        std::cout << "* Log settings:\n"
             << "\t- Export Tangle: " << bool(honestModule->par("exportTangle")) << "\n"
             << "\t- Export tips number: " << bool(honestModule->par("exportTipsNumber")) << "\n"
             << "\t- Delete tips number logs before: " << bool(honestModule->par("wipeLogTipsNumber")) << "\n";
    }

    if(maliciousModule)
    {
        std::cout << "\n----------- Malicious node settings -----------\n"
                  << "* Attack stage: " << double(maliciousModule->par("attackStage")) << "\n";

        bool PCattack = maliciousModule->par("PCattack");
        bool SPattack = maliciousModule->par("SPattack");

        if(PCattack)
        {
            std::cout << "* Parasite chain attack settings:\n"
                      << "\t- The computing power of the node: " << double(maliciousModule->par("propComputingPower")) << "\n"
                      << "\t- The proportion of transactions in the chain: " << double(maliciousModule->par("propChainTips")) << "\n";
        }

        if(SPattack)
        {
            std::cout << "* Splitting attack settings:\n"
                      << "\t- The value to set the size of the branches at initialization: " << double(maliciousModule->par("sizeBrancheProp")) << "\n"
                      << "\t- The computing power of the node when maintaining the branches: " << double(maliciousModule->par("propRateMB")) << "\n";
        }

        std::cout << "* Log settings:\n"
             << "\t- Export the conflicted transactions: " << bool(maliciousModule->par("exportConflictTx")) << "\n"
             << "\t- Export the resilience of the parasite chain attack: " << bool(maliciousModule->par("exportDiffTxChain")) << "\n"
             << "\t- Delete parasite chain attack logs before: " << bool(maliciousModule->par("wipeLogTipsNumber")) << "\n"
             << "\t- Export the resilience of the splitting attack: " << bool(maliciousModule->par("exportBranchSize")) << "\n"
             << "\t- Delete splitting attack logs before: " << bool(maliciousModule->par("wipeLogBranchSize")) << "\n";
    }
}

void ConfiguratorModule::initialize()
{
    EV << "Setting up connections:" << std::endl;

    int currentRun = getEnvir()->getConfigEx()->getActiveRunNumber();

    auto myTopo = getEdgeList();
    auto myModules = getModules();
    auto myRNG = randomNumberGenerator(currentRun,1.0);

    for(const auto& edge : myTopo)
    {
        checkError(edge.first,myModules.size());
        auto moduleOut = myModules[edge.first];

        for(int idx : edge.second)
        {
            checkError(idx,myModules.size());
            auto moduleIn = myModules[idx];
            connectModules(moduleOut,moduleIn,myRNG);
            EV << "Module " << moduleOut->getId() - 3 << " ---> Module " << moduleIn->getId() - 3 << std::endl;
        }
    }

    logConfig();
}
