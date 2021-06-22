//includes
#pragma once
#include <array>
#include <vector>
#include <memory>
#include <map>
#include <utility>
#include <random>
#include <ctime>
#include <cmath>
#include <string>
#include <omnetpp.h>

struct Site; //a transaction
class Node; //a node
class RNG; //for the RNG

using pTr_S = Site*;
using VpTr_S = std::vector<pTr_S>;

struct Site
{
    //the ID of the transaction
    std::string ID;

    //The node that issued this transaction
    Node* issuedBy;

    //bool
    bool isGenesisBlock = false;
    bool isApproved = false;

    //used during the recursion weight compute process
    bool isVisited = false;

    //number of walks taken to select this transaction during the random walk
    int walkBacktracks;

    //Transactions that have approved this transaction directly
    VpTr_S approvedBy;

    //Transactions approved by this transaction directly
    VpTr_S S_approved;

    //Simulation time when the transaction was issued - set by the Node
    omnetpp::simtime_t issuedTime;

    //Simulation time when this transaction ceased to be a tip (i.e has been approved)
    omnetpp::simtime_t approvedTime;

};

class Node
{
    private:
    RNG * RNGPtr = nullptr;

    public:
        //ID of the Node
        std::string ID;

        //Local Tangle
        VpTr_S myTangle;

        //Keep a record of all the current unapproved transactions
        std::map<int,pTr_S> myTips;

        //The first transaction
        pTr_S genesisBlock;

        //allow to check if the node deleted his Tangle
        bool ifDeleted = false;

        //setter and getter for RNG
        RNG* getRNGPtr() const;
        void setRNGPtr(RNG* rng);

        //Returns ref to the RNG, used in all Nodes methods
        std::mt19937& getRandGen();

        //check if the node can issue a transaction using prob para in NodeModule
        bool ifIssue(double prob);

        //generate a new transaction
        pTr_S createSite(std::string ID);

        //generate the Genesis Block;
        pTr_S createGenBlock();

        //free memory used by myTangle
        void DeleteTangle(Node &myNode, VpTr_S &myTangle);

        //tracking
        void printTangle(VpTr_S myTangle, pTr_S Gen);
        void printTipsLeft(int numberTips);

        //Returns a copy of the current tips from the Tangle
        std::map<int,pTr_S> giveTips();

        //Random Walk
        pTr_S RandomWalk(pTr_S start, double alphaVal, std::map<int,pTr_S>& tips, omnetpp::simtime_t timeStamp, int &walk_time);

        //TSA
        VpTr_S IOTA(double alphaVal, std::map<int,pTr_S>& tips, omnetpp::simtime_t timeStamp, int W, int N);
        VpTr_S GIOTA(double alphaVal, std::map<int,pTr_S>& tips, omnetpp::simtime_t timeStamp, int W, int N);
        VpTr_S EIOTA(double p1, double p2, std::map<int,pTr_S>& tips, omnetpp::simtime_t timeStamp, int W, int N);

        //Compute Weight
        int _computeWeight(VpTr_S& visited, pTr_S& current, omnetpp::simtime_t timeStamp );
        int ComputeWeight(pTr_S tr, omnetpp::simtime_t timeStamp);

        //Back track for selecting start sites for the random walk
        pTr_S getWalkStart(std::map<int,pTr_S>& tips, int backTrackDist);

        //filter view due to network asynchronicity
        bool isRelativeTip(pTr_S& toCheck, std::map<int,pTr_S>& tips);
        void filterView(VpTr_S& view, omnetpp::simtime_t timeStamp);

        //remove newly confirmed tips from myTips;
        void ReconcileTips(const VpTr_S& removeTips, std::map<int,pTr_S>& myTips);

        //creates a new transaction, selects tips for it to approve, then adds the new transaction to the tip map
        pTr_S attach(Node& myNode, std::string ID, omnetpp::simtime_t attachTime, VpTr_S& chosen);

        //update the local tangle when a new transaction is received
        void updateTangle(pTr_S toAdd, omnetpp::simtime_t attachTime);
};

class RNG
{
    private:
    // RNG in tangle to simplify tip selection
    std::mt19937 tipSelectGen;

    public:
    //Returns ref to the RNG, used in all Nodes methods
    std::mt19937& getRandGen();
};
