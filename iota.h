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
struct MsgUpdate; //a message to send to others nodes to update their tangles

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

class Node : public omnetpp::cSimpleModule
{
    public:
        //ID of the Node
        std::string ID;

        //Local Tangle
        VpTr_S myTangle;

        //Keep a record of all the current unapproved transactions
        std::map<std::string,pTr_S> myTips;

        //The first transaction
        pTr_S genesisBlock;

        //allow to check if the node deleted his Tangle
        bool ifDeleted = false;

        //RNG between min and max
        int rangeRandom(int min, int max);

        //check if the node can issue a transaction using prob para in NodeModule
        bool ifIssue(double prob);

        //generate a new transaction
        pTr_S createSite(std::string ID);

        //generate the Genesis Block;
        pTr_S createGenBlock();

        //free memory used by myTangle
        void DeleteTangle(Node &myNode, VpTr_S &myTangle);

        //tracking
        void printTangle(VpTr_S myTangle, int NodeID);
        void printTipsLeft(int numberTips, int NodeID);

        //Returns a copy of the current tips from the Tangle
        std::map<std::string,pTr_S> giveTips();

        //Random Walk
        pTr_S RandomWalk(pTr_S start, double alphaVal, std::map<std::string,pTr_S>& tips, omnetpp::simtime_t timeStamp, int &walk_time);

        //TSA
        VpTr_S IOTA(double alphaVal, std::map<std::string,pTr_S>& tips, omnetpp::simtime_t timeStamp, int W, int N);
        VpTr_S GIOTA(double alphaVal, std::map<std::string,pTr_S>& tips, omnetpp::simtime_t timeStamp, int W, int N);
        VpTr_S EIOTA(double p1, double p2, std::map<std::string,pTr_S>& tips, omnetpp::simtime_t timeStamp, int W, int N);

        //Compute Weight
        int _computeWeight(VpTr_S& visited, pTr_S& current, omnetpp::simtime_t timeStamp );
        int ComputeWeight(pTr_S tr, omnetpp::simtime_t timeStamp);

        //Back track for selecting start sites for the random walk
        pTr_S getWalkStart(std::map<std::string,pTr_S>& tips, int backTrackDist);

        //filter view due to network asynchronicity
        bool isRelativeTip(pTr_S& toCheck, std::map<std::string,pTr_S>& tips);
        void filterView(VpTr_S& view, omnetpp::simtime_t timeStamp);

        //remove newly confirmed tips from myTips;
        void ReconcileTips(const VpTr_S& removeTips, std::map<std::string,pTr_S>& myTips);

        //creates a new transaction, selects tips for it to approve, then adds the new transaction to the tip map
        pTr_S attach(std::string ID, omnetpp::simtime_t attachTime, VpTr_S& chosen);

        //update the local tangle when a new transaction is received
        void updateTangle(MsgUpdate* Msg, omnetpp::simtime_t attachTime);
};

class NodeModule : public omnetpp::cSimpleModule
{
    public:
        std::string ShortId;
        int trLimit;
        double prob;
        int NeighborsNumber;
        Node self;
        int TrCount;
        int stopCount;
        omnetpp::simtime_t powTime;

    protected:
        virtual void initialize() override;
        virtual void handleMessage(omnetpp::cMessage * msg) override;
};

struct MsgUpdate
{
    //the ID of transaction
    std::string ID;

    //transactions approuved by this transaction
    std::vector<std::string> S_approved;

    //The node that issued this transaction
    Node* issuedBy;

    //Simulation time when the transaction was issued - set by the Node
    omnetpp::simtime_t issuedTime;
};
