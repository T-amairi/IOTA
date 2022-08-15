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

    if(!file.is_open())
    {
        throw std::runtime_error("Could not open CSV file");
    }

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

void ConfiguratorModule::connectModules(cModule* moduleOut, cModule* moduleIn)
{
    auto channel = cDelayChannel::create("Channel");

    if(getParentModule()->par("ifRandDelay"))
    {
       channel->setDelay(uniform(minDelay,maxDelay));
    }

    else
    {
        channel->setDelay(delay);
    }

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
         << "* Selected topology: " << network->par("topology").str() << "\n"
         << "* Number of malicious nodes: " << network->par("nbMaliciousNode").intValue() << "\n"
         << "* Number of honest nodes: " << network->par("nbHonestNode").intValue() << "\n";

    bool ifRandDelay = network->par("ifRandDelay");
    std::cout << "* To set up a random delay based on an interval: " << ifRandDelay << "\n";

    if(ifRandDelay)
    {
        cPar& parMinDelay = network->par("minDelay");
        cPar& parMaxDelay = network->par("maxDelay");

        std::cout << "\t- Minimum delay: " << minDelay << " " << parMinDelay.getUnit() << "\n";
        std::cout << "\t- Maximum delay: " << maxDelay << " " << parMaxDelay.getUnit() << "\n";
    }

    else
    {
        cPar& parDelay = network->par("delay");
        std::cout << "\t- Fixed delay: " << delay << " " << parDelay.getUnit() << "\n";
    }

    if(honestModule)
    {
        cPar& parRateMean = honestModule->par("rateMean");
        cPar& parPowTime = honestModule->par("powTime");

        std::cout << "\n------------ Honest node settings ------------\n"
             << "* The exponential distribution mean of the issuing rate: " << parRateMean.doubleValue() << " per " << parRateMean.getUnit() << "\n"
             << "* Proof of work time: " << parPowTime.doubleValue() << " " << parPowTime.getUnit() << "\n"
             << "* Transaction limit: " << honestModule->par("transactionLimit").intValue() << "\n";

        std::string TSA = honestModule->par("TSA");
        std::cout << "* Tips selection algorithm: " << TSA << "\n"
             << "\t- WProp: " << honestModule->par("WProp").doubleValue() << "\n"
             << "\t- WPercentageThreshold: " << honestModule->par("WPercentageThreshold").doubleValue() << "\n"
             << "\t- N: " << int(honestModule->par("N")) << "\n";

        if(TSA == "IOTA" || TSA == "GIOTA")
        {
            std::cout << "\t- Alpha: " << honestModule->par("alpha").doubleValue() << "\n";

            if(TSA == "GIOTA")
            {
                std::cout << "\t- confDistance: " << honestModule->par("confDistance").intValue() << "\n";
            }
        }

        else
        {
            std::cout << "\t- p1: " << honestModule->par("p1").doubleValue() << "\n"
                 << "\t- p2: " << honestModule->par("p2").doubleValue() << "\n"
                 << "\t- Low alpha: " << honestModule->par("lowAlpha").doubleValue() << "\n"
                 << "\t- High alpha: " << honestModule->par("highAlpha").doubleValue() << "\n";
        }

        std::cout << "* Log settings:\n"
             << "\t- Export Tangle: " << honestModule->par("exportTangle").boolValue() << "\n"
             << "\t- Export tips number: " << honestModule->par("exportTipsNumber").boolValue() << "\n"
             << "\t- Delete tips number logs before: " << honestModule->par("wipeLogTipsNumber").boolValue() << "\n";
    }

    if(maliciousModule)
    {
        std::cout << "\n----------- Malicious node settings -----------\n"
                  << "* Attack stage: " << maliciousModule->par("attackStage").doubleValue() << "\n";

        bool PCattack = maliciousModule->par("PCattack");
        bool SPattack = maliciousModule->par("SPattack");

        if(PCattack)
        {
            std::cout << "* Parasite chain attack settings:\n"
                      << "\t- The computing power of the node: " << maliciousModule->par("propComputingPower").doubleValue() << "\n"
                      << "\t- The proportion of transactions in the chain: " << maliciousModule->par("propChainTips").doubleValue() << "\n";
        }

        if(SPattack)
        {
            std::cout << "* Splitting attack settings:\n"
                      << "\t- The value to set the size of the branches at initialization: " << maliciousModule->par("sizeBrancheProp").doubleValue() << "\n"
                      << "\t- The computing power of the node when maintaining the branches: " << maliciousModule->par("propRateMB").doubleValue() << "\n";
        }

        std::cout << "* Log settings:\n"
             << "\t- Export the conflicted transactions: " << maliciousModule->par("exportConflictTx").boolValue() << "\n"
             << "\t- Export the resilience of the parasite chain attack: " << maliciousModule->par("exportDiffTxChain").boolValue() << "\n"
             << "\t- Delete parasite chain attack logs before: " << maliciousModule->par("wipeLogTipsNumber").boolValue() << "\n"
             << "\t- Export the resilience of the splitting attack: " << maliciousModule->par("exportBranchSize").boolValue() << "\n"
             << "\t- Delete splitting attack logs before: " << maliciousModule->par("wipeLogBranchSize").boolValue() << "\n";
    }
}

void ConfiguratorModule::initialize()
{
    EV << "Setting up connections:" << "\n";

    auto myTopo = getEdgeList();
    auto myModules = getModules();

    delay = getParentModule()->par("delay").doubleValue();
    minDelay = getParentModule()->par("minDelay").doubleValue();
    maxDelay = getParentModule()->par("maxDelay").doubleValue();

    for(const auto& edge : myTopo)
    {
        checkError(edge.first,myModules.size());
        auto moduleOut = myModules[edge.first];

        for(int idx : edge.second)
        {
            checkError(idx,myModules.size());
            auto moduleIn = myModules[idx];
            connectModules(moduleOut,moduleIn);
            EV << "Module " << moduleOut->getId() - 3 << " ---> Module " << moduleIn->getId() - 3 << "\n";
        }
    }

    logConfig();
}
