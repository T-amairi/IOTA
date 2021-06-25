//includes
#pragma once
#include <array>
#include <vector>
#include <memory>
#include <map>
#include <utility>
#include <cmath>
#include <omnetpp.h>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <stdio.h>
#include <string>
#include <numeric>
#include <fstream>
#include <sstream>
#include <functional>

struct Site; //a transaction
class NodeModule; //a node
struct MsgUpdate; //a message to send to others nodes to update their tangles

using pTr_S = Site*;
using VpTr_S = std::vector<pTr_S>;

using namespace omnetpp;

struct Site
{
    //the ID of the transaction
    std::string ID;

    //The node that issued this transaction
    NodeModule* issuedBy;

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
    simtime_t issuedTime;

    //Simulation time when this transaction ceased to be a tip (i.e has been approved)
    simtime_t approvedTime;

};

class NodeModule : public cSimpleModule
{
    public:
        //ID of the Node
        std::string ID;

        //timers
        cMessage * msgIssue;
        cMessage * msgUpdate;
        MsgUpdate * Msg;

        //RNG between min and max using omnet
        int rangeRandom(int min, int max);

        //check if the node can issue a transaction using prob para in NodeModule
        bool ifIssue(double prob);

        //generate a new transaction
        pTr_S createSite(std::string ID);

        //generate the Genesis Block;
        pTr_S createGenBlock();

        //free memory used by myTangle
        void DeleteTangle();

        //tracking
        void printTangle();
        void printTipsLeft();

        //Returns a copy of the current tips from the Tangle
        std::map<std::string,pTr_S> giveTips();

        //Random Walk
        pTr_S RandomWalk(pTr_S start, double alphaVal, std::map<std::string,pTr_S>& tips, simtime_t timeStamp, int &walk_time);

        //TSA
        VpTr_S IOTA(double alphaVal, std::map<std::string,pTr_S>& tips, simtime_t timeStamp, int W, int N);
        VpTr_S GIOTA(double alphaVal, std::map<std::string,pTr_S>& tips, simtime_t timeStamp, int W, int N);
        VpTr_S EIOTA(double p1, double p2, std::map<std::string,pTr_S>& tips, simtime_t timeStamp, int W, int N);

        //Compute Weight
        int _computeWeight(VpTr_S& visited, pTr_S& current, simtime_t timeStamp );
        int ComputeWeight(pTr_S tr, simtime_t timeStamp);

        //Back track for selecting start sites for the random walk
        pTr_S getWalkStart(std::map<std::string,pTr_S>& tips, int backTrackDist);

        //filter view due to network asynchronicity
        bool isRelativeTip(pTr_S& toCheck, std::map<std::string,pTr_S>& tips);
        void filterView(VpTr_S& view, simtime_t timeStamp);

        //remove newly confirmed tips from myTips;
        void ReconcileTips(const VpTr_S& removeTips, std::map<std::string,pTr_S>& myTips);

        //creates a new transaction, selects tips for it to approve, then adds the new transaction to the tip map
        pTr_S attach(std::string ID, simtime_t attachTime, VpTr_S& chosen);

        //update the local tangle when a new transaction is received
        void updateTangle(MsgUpdate* Msg, simtime_t attachTime);

        //to debug only
        void debug();

    private:
        //how many transactions the node can issue (set in NED file)
        int txLimit;

        //the probability to issue a new transaction (set in NED file)
        double prob;

        //number of adjacent nodes
        int NeighborsNumber;

        //counts the number of transactions issued by the node
        int txCount = 0;

        //PoW
        simtime_t powTime;

        //Local Tangle
        VpTr_S myTangle;

        //Keep a record of all the current unapproved transactions
        std::map<std::string,pTr_S> myTips;

        //buffer containing the transactions to be updated
         //myBuffer;

        //The first transaction
        pTr_S genesisBlock;

    protected:
        virtual void initialize() override;
        virtual void handleMessage(cMessage * msg) override;
        virtual void finish() override;
};

struct MsgUpdate
{
    //the ID of the transaction
    std::string ID;

    //transactions approved by this transaction
    std::vector<std::string> S_approved;

    //The node that issued this transaction
    NodeModule* issuedBy;
};
