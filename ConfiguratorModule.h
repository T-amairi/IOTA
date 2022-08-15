//includes
#pragma once
#include <fstream>
#include "Structures.h"

//ConfiguratorModule : non-networked module setting up the topology
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

        //Throw an error if there is a conflict between the CSV file and the parameters in the NED file (e.g number of modules)
        void checkError(int nodeID, int totalModules) const;

        //Log the current configuration
        void logConfig() const;

        //Overriding the initialize() function from the cSimpleModule class
        void initialize() override;

    private:
        //delays
        double delay;
        double minDelay;
        double maxDelay;
};

Define_Module(ConfiguratorModule);
