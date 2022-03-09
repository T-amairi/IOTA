//includes
#pragma once
#include <omnetpp.h>
#include <string>
#include <vector>
#include <fstream>
#include <map>

using namespace omnetpp;

class ConfiguratorModule : public cSimpleModule
{
    public:
        //Split a line (from the CSV file) in a list of int according to the delimeter ','
        std::vector<int> split(const std::string& line) const;
        //Connect two modules
        void connectModules(cModule* moduleOut, cModule* moduleIn);

        //Return the edge list (using a map : [Node (key)] -> Adjacent nodes (value)) of the used topology from the csv file in the topologies folder
        std::map<int,std::vector<int>> getEdgeList() const;
        //Return a vector containing pointers for all modules (Honest and Malicious)
        std::vector<cModule*> getModules() const;

        //throw an error if there is a conflict between the CSV file and the parameters in the NED file (e.g number of modules)
        void checkError(int nodeID, int totalModules) const;

        //Overriding the initialize() function from the cSimpleModule class
        void initialize() override;
};

Define_Module(ConfiguratorModule);
