//includes
#pragma once
#include <fstream>
#include <algorithm>
#include <numeric>

#include "Structures.h"

//Abstract class for the HonestModule class & MaliciousModule class
class AbstractModule : public cSimpleModule
{
    public:
        Tx* createTx(); //generate a new transaction
        void createGenBlock(); //generate the Genesis block;

        //getter 
        int getTxCount() const;
        int getTxLimit() const;

        //setter
        void setTxLimit(int newLimit);

        //track data
        void printTangle() const; //export the Tangle in CSV format to be drawned with the python script 
        void printTipsLeft(bool ifDeleteFile) const; //export the amount of tips at the end of the simulation in a CSV file
        void printStats(bool ifDeleteFile = false) const; //write in a CSV file some stats of the simulation at a given time (i.e when the function is called) 

        /* return a vector of tuple containing:
            - the pointer of the selected tip after the random walk
            - the number of time that this tip has been selected
            - its first walk time during the random walk
        */
        std::vector<std::tuple<Tx*,int,int>> getSelectedTips(double alphaVal, int W, int N);

        //TSA
        VpTx IOTA(double alphaVal, int W, int N);
        VpTx GIOTA(double alphaVal, int W, int N);
        VpTx EIOTA(double p1, double p2, double lowAlpha, double highAlpha, int W, int N);

        //return tips using the TSA method set in the NED file 
        VpTx getTipsTSA();

        //random walk based on a Markov Chain Monte Carlo (MCMC)
        Tx* weightedRandomWalk(Tx* startTx, double alphaVal, int &walkTime);
        Tx* randomWalk(Tx* startTx, int &walkTime);

        //select start site for the random walk
        Tx* getWalkStart(int backTrackDist) const;

        void unionConflictTx(Tx* theSource, Tx* toUpdate); //get the union between two conflictTx attribute
        bool isLegitTip(Tx* tip) const; //check if a tip is legit (i.e do not approves conflicted transaction)
        bool ifConflictedTips(Tx* tip1, Tx* tip2) const; //check if there is a conflict between two tips

        //update the conflictTx attribute of a transaction
        void updateConflictTx(Tx* startTx);
        void _updateconflictTx(Tx* currentTx, VpTx& visitedTx, std::string conflictID);

        //check if a transaction approves directly or indirectly an another transaction 
        bool isApp(Tx* startTx, std::string idToCheck);
        void _isapp(Tx* currentTx, VpTx& visitedTx, std::string idToCheck, bool& res);

        //compute the weight of a transaction
        int computeWeight(Tx* tx);
        int _computeweight(Tx* currentTx, VpTx& visitedTx);
        
        //compute the confidence for each transaction
        void computeConfidence(Tx* startTx, double conf);
        void _computeconfidence(Tx* currentTx, VpTx& visitedTx, int distance, double conf);

        //compute the avg confidence of a tip
        void getAvgConf(Tx* startTx, double& avg);
        void _getavgconf(Tx* currentTx, VpTx& visitedTx, int distance, double& avg);

        bool isPresent(std::string txID) const; //check if a transaction is already present in the myTangle vector
        Tx* attachTx(simtime_t attachTime, VpTx chosenTips); //creates a new transaction, then adds the new transaction to the tip

        Tx* updateTangle(const dataUpdate* data); //update the local tangle when a new transaction is received
        void updateMyTips(std::string tipID); //remove newly confirmed tips from myTips
        void updateMyBuffer(); //remove conflicted transaction from the buffer if founded

        void spreadTx(cModule* senderModule, dataUpdate* data); //broadcast to the neighbors modules the received transaction
        void broadcastTx(const Tx* newTx); //broadcast to the neighbors modules the issued transaction

        //free the used memory
        void deleteTangle();

        void _initialize(); //to call in the initialize() function from cSimpleModule
        void _finish(bool exportTangle, std::pair<bool,bool> exportTipsNumber); //to call in the finish() function from cSimpleModule

        //functions used when receiving a specified message type (in handleMessage()) 
        virtual void caseISSUE() = 0;
        virtual void casePOW(cMessage* msg) = 0;
        virtual void caseUPDATE(cMessage* msg) = 0;

    protected:
        std::string ID; //the ID of the module
        int txCount = 0; //counts the number of transactions issued by the module
        int txLimit; //how many transactions the module can issue (set in NED file)
        int WThreshold; //the maximum limit that W can take (check the NED file for more precision)

        simtime_t rateMean; //exponential distribution with the given mean (set in the NED file)
        simtime_t powTime; //PoW time (set in NED file)

        cMessage* msgIssue; //to issue a new transaction
        cMessage* msgPoW; //to wait during the pow time
        cMessage* msgUpdate; //to send to others modules
        
        randomNumberGenerator* myRNG; //rng class for the uniform & exponential distribution 
        VpTx myTangle; //local Tangle
        VpTx myTips; //keep a record of all the current unapproved transactions
        std::vector<std::string> myBuffer; //a buffer for all conflicted transaction to be checked  
};
