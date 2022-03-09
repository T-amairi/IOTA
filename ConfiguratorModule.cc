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

void ConfiguratorModule::connectModules(cModule* moduleOut, cModule* moduleIn)
{
    double delay = 0.0;

    if(getParentModule()->par("ifRandDelay"))
    {
       double minDelay = getParentModule()->par("minDelay");
       double maxDelay = getParentModule()->par("maxDelay");
       delay = uniform(minDelay,maxDelay);
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

void ConfiguratorModule::initialize()
{
    EV << "Setting up connections:" << std::endl;

    auto myTopo = getEdgeList();
    auto myModules = getModules();

    for(auto const edge : myTopo)
    {
        checkError(edge.first,myModules.size());
        auto moduleOut = myModules[edge.first];

        for(int idx : edge.second)
        {
            checkError(idx,myModules.size());
            auto moduleIn = myModules[idx];
            connectModules(moduleOut,moduleIn);
            EV << "Module " << moduleOut->getId() - 3 << " ---> Module " << moduleIn->getId() - 3 << std::endl;
        }
    }
}
