//includes
#pragma once
#include "AbstractModule.h"

class MaliciousModule : public AbstractModule
{
    public:
        //to handle the attack
        bool ifLaunchAttack() const; //check if we can launch an attack based on the parameter attackStage set in the ned file
        bool ifModulesFinished() const;  //check if the other modules have finished (i.e txCount > txLimit) to stop the simulation during a splitting attack
        void setTxLimitModules(); //set a new txLimit value for the other modules

        //track attacks
        void printConflictTx();
        void printBranchSize(bool ifDeleteFile);
        void printDiffTxChain(bool ifDeleteFile);
        
        //Parasite Chain Attack
        Tx* getTargetTx() const; //to get the transaction that will be double spend 
        Tx* getRootTx(simtime_t issuedTimeTarget); //to get the root of the chain
        void linkChain(Tx* tx1, Tx* tx2) const; //to link two tx in the chain
        VpTx getParasiteChain(Tx* rootTx, std::string targetID, int chainLength, int tipsCount); //to get the chain

        //Splitting Attack
        void iniSPA(); //to build the two branches in offline
        void mergeAndBroadcast(); //adds transactions in myTips & myTangle, then broadcasts them
        void updateBranches(Tx* tip); //updates branches when a new transaction is received
        void handleBalance(); //handle the balance between the two branches when receiving a new transaction 
        Tx* maintainBalance(); //maintain the balance between the two tranches

        //functions called in handleMessage()
        void caseATTACK();
        void casePCA();
        void caseSPA();
        void caseMB(cMessage* msg);

        //overridden functions from AbstractModule class called in handleMessage() function
        void caseISSUE() override;
        void casePOW(cMessage* msg) override;
        void caseUPDATE(cMessage* msg) override;

        //overridden functions from cSimpleModule class
        void initialize() override;
        void handleMessage(cMessage * msg) override;
        void finish() override;
    
    private:
        double attackStage; //when initiate an attack (set in the NED file)
        simtime_t rateMB; //equivalent to "rateMean" but for the splitting attack (power of the malicious) 
        double sizeBrancheProp; //sizes of the branches at ini  (set in the NED file)
        int totalModules; //number of modules present in the simulation

        int countMB; //a counter to track the number of transaction issued (during a splitting attack)
        bool cachedRequest; //allow to cache the request of maintaining balance 
        int whichBranch; //to know which branch to maintain (during a splitting attack)
        bool ifSimFinished; //to know if the other modules have finished i.e txCount >= txLimit (during a splitting attack) 

        cMessage* msgAttack; //to initiate an attack
        cMessage* msgParasiteChain = nullptr; //to initiate a parasite chain attack
        cMessage* msgSplitting = nullptr; //to initiate a splitting attack
        cMessage* msgMaintainBalance = nullptr; //to maintain balance

        VpTx myDoubleSpendTx; //keep a record of all double spend transaction (starting with a "-")
        VpTx myBranch1; //the first branch of the splitting attack
        VpTx myBranch2; //the second branch of the splitting attack
};    

Define_Module(MaliciousModule);
