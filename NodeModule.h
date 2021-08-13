//includes
#pragma once
#include <array>
#include <vector>
#include <memory>
#include <map>
#include <utility>
#include <cmath>
#include <cstdlib>
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
#include <unordered_set>

struct Site; //a transaction
class NodeModule; //a node
struct MsgUpdate; //a message to send to others nodes to update their tangles
struct MsgPoW; //pow timer

using pTr_S = Site*;
using VpTr_S = std::vector<pTr_S>;

using namespace omnetpp;

struct Site
{
    //the ID of the transaction
    std::string ID;

    //The node that issued this transaction
    NodeModule* issuedBy;

    //confidence (for G-IOTA)
    double confidence = 0.0;

    //how much a tip has been selected during a TSA (for G-IOTA)
    int countSelected = 0;

    //bool
    bool isGenesisBlock = false;
    bool isApproved = false;

    //used during the recursion weight compute process
    bool isVisited = false;

    //Transactions that have approved this transaction directly
    VpTr_S approvedBy;

    //Transactions approved by this transaction directly
    VpTr_S S_approved;

    //Simulation time when the transaction was issued - set by the Node
    simtime_t issuedTime;

    //Simulation time when this transaction ceased to be a tip (i.e has been approved)
    simtime_t approvedTime;

    //contain the site ID and all transactions ID approved directly or indirectly by it
    std::unordered_set<std::string> pathID;

};

class NodeModule : public cSimpleModule
{
    public:
        //ID of the Node
        std::string ID;

        //counts the number of transactions issued by the node
        int txCount = 0;

        //how many transactions the node can issue (set in NED file)
        int txLimit;

        //timers
        cMessage * msgIssue;
        cMessage * msgPoW;
        cMessage * msgUpdate;

        //data sent
        MsgPoW * MsgP;

        //generate a new transaction
        pTr_S createSite(std::string ID);

        //generate the Genesis Block;
        pTr_S createGenBlock();

        //free memory used by myTangle
        void DeleteTangle();

        //tracking
        void printTangle();
        void printTipsLeft();
        void stats();
        void printChain();
        void PercentDiffBranch();
        void DiffTxChain();

        //Random walk based on a MCMC
        pTr_S WeightedRandomWalk(pTr_S start, double alphaVal, int &walk_time);
        pTr_S RandomWalk(pTr_S start, int &walk_time);

        //give the right pathID without any duplicates
        std::unordered_set<std::string> getpathID(VpTr_S chosenTips);

        //find if a tip is legit (i.e if it approves at the same time two conflicted transactions)
        std::tuple<bool,std::string> IfLegitTip(std::unordered_set<std::string> path);

        //check if two transactions are in conflict
        bool IfConflict(std::tuple<bool,std::string> tup1, std::tuple<bool,std::string> tup2, std::unordered_set<std::string> path1, std::unordered_set<std::string> path2);

        //get confidence for one site (to resolve conflict)
        int getConfidence(std::string id);

        //TSA
        VpTr_S IOTA(double alphaVal, int W, int N);
        VpTr_S GIOTA(double alphaVal, int W, int N);
        VpTr_S EIOTA(double p1, double p2, double lowAlpha, double highAlpha, int W, int N);

        //Compute Weight
        int _computeWeight(VpTr_S& visited, pTr_S& current);
        int ComputeWeight(pTr_S tr);

        //select start site for the random walk
        pTr_S getWalkStart(int backTrackDist);

        //remove newly confirmed tips from myTips;
        void ReconcileTips(const VpTr_S& removeTips, std::map<std::string,pTr_S>& myTips);

        //creates a new transaction, selects tips for it to approve, then adds the new transaction to the tip map
        pTr_S attach(std::string ID, simtime_t attachTime, VpTr_S& chosen);

        //check if we have conflict in the buffer & myTangle vectors
        bool IfPresent(std::string txID);

        //check if we can submit a new transaction in the Tangle
        bool ifAddTangle(std::vector<std::string> S_approved);

        //update the buffer by deleting transactions that we can add to the tangle
        void updateBuffer();

        //update the local tangle when a new transaction is received
        void updateTangle(MsgUpdate* Msg);

        //read csv file to connect modules (for ws & exp topo)
        std::vector<int> readCSV(bool IfExp);

        //check if we can launch an attack based on the par AttackStage in the ned file
        bool IfAttackStage();

        //build the parasite chain
        VpTr_S getParasiteChain(pTr_S RootTip, std::string TargetID, int ChainLength, int NbTipsChain);

        //build the two branches in conflict (splitting attack scenario)
        void iniSplittingAttack();

        //update the branches when a new transaction is received
        void updateBranches(pTr_S newTx);

        //Maintain the balance between the two branches
        pTr_S MaintainingBalance(int whichBranch);

        //check if the other nodes have finished (i.e txCount > txLimit) to stop the sim during a splitting attack
        void IfNodesfinished();

    private:
        //exponential distribution with the given mean (that is, with parameter lambda=1/mean).
        simtime_t mean;

        //number of adjacent nodes
        int NeighborsNumber;

        //number of nodes present in the simulation
        int NodeModuleNb = 0;

        //for attack scenarios (to avoid performing twice)
        bool IfAttack = false;

        //to control the splitting attack (e.g resume & stop)
        bool IfAttackSP = false;

        //to check if the splitting attack initialization has finished
        bool IfIniSP = true;

        //a buffer to know if you have to check the balance (during a splitting attack)
        bool IfScheduleMB = false;

        //to know if the other nodes have finished i.e txCount >= txLimit (during a splitting attack)
        bool IfSimFinished = false;

        //a counter to track the number of transaction created (during a splitting attack)
        int countMB = 0;

        //to know which branch to maintain (during a splitting attack)
        int whichBranch;

        //PoW
        simtime_t powTime;

        //Local Tangle
        VpTr_S myTangle;

        //keep a record of all double spend transaction (starting with a "-")
        VpTr_S myDoubleSpendTx;

        //keep a record of the transactions present in the branches (SplittingAttack)
        VpTr_S branch1;
        VpTr_S branch2;

        //Keep a record of all the current unapproved transactions
        std::map<std::string,pTr_S> myTips;

        //buffer containing the transactions to be updated
        std::vector<MsgUpdate*> myBuffer;

        //The first transaction
        pTr_S genesisBlock;

    protected:
        virtual void initialize() override;
        virtual void handleMessage(cMessage * msg) override;
        virtual void finish() override;
};

struct MsgPoW
{
    //the ID of the tip
    std::string ID;

    //chosen tips from TSA
    VpTr_S chosen;
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
