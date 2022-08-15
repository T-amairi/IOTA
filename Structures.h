//includes 
#pragma once
#include <omnetpp.h>
#include <omp.h>

using namespace omnetpp;

struct Tx;
using VpTx = std::vector<Tx*>;
enum MessageType{ISSUE,POW,UPDATE,ATTACK,PCA,SPA,MB};

//a transaction
struct Tx
{
    Tx(std::string _ID):ID(_ID) //constructor
    {
        for(int i = 0; i < omp_get_num_procs(); i++)
        {
            isVisited.push_back(false);
            confidence.push_back(0.0);
            countSelected.push_back(0);
        }
    };

    const std::string ID; //the ID of the transaction
    
    std::vector<double> confidence; //confidence (for G-IOTA) (each idx is associated with a thread based on his ID)
    std::vector<int> countSelected; //how much a tip has been selected during a TSA (for G-IOTA) (each idx is associated with a thread based on his ID)

    bool isGenesisBlock = false;
    bool isApproved = false; 
    std::vector<bool> isVisited; //used during the recursion weight compute process (each idx is associated with a thread based on his ID)
    
    VpTx approvedBy; //transactions that have approved this transaction directly
    VpTx approvedTx; //transactions approved by this transaction directly

    simtime_t issuedTime; //simulation time when the transaction was issued
    simtime_t approvedTime; //simulation time when this transaction ceased to be a tip (i.e has been approved)

    std::map<std::string,std::pair<bool,bool>> conflictTx; //stores conflicted transaction approved by this transaction
};

//data to send to others modules to update their tangles
struct dataUpdate
{
    std::string ID;  //the ID of the transaction
    std::vector<std::string> approvedTx; //transactions approved by this transaction
};
