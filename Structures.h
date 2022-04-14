//includes 
#pragma once
#include <omnetpp.h>

using namespace omnetpp;

struct Tx;
using VpTx = std::vector<Tx*>;
enum MessageType{ISSUE,POW,UPDATE,ATTACK,PCA,SPA,MB};

//a transaction
struct Tx
{
    Tx(std::string _ID):ID(_ID){}; //constructor

    const std::string ID; //the ID of the transaction
    
    double confidence = 0.0; //confidence (for G-IOTA)
    int countSelected = 0; //how much a tip has been selected during a TSA (for G-IOTA)

    bool isGenesisBlock = false;
    bool isApproved = false; 
    bool isVisited = false; //used during the recursion weight compute process
    
    VpTx approvedBy; //transactions that have approved this transaction directly
    VpTx approvedTx; //transactions approved by this transaction directly

    simtime_t issuedTime; //simulation time when the transaction was issued
    simtime_t approvedTime; //simulation time when this transaction ceased to be a tip (i.e has been approved)

    std::map<std::string,std::pair<bool,bool>> conflictTx; //stores conflicted transaction approved by this transaction
    std::map<int,bool> isVisitedByThread; //used during the recursion weight compute process (OpenMP implementation)
};

//data to send to others modules to update their tangles
struct dataUpdate
{
    std::string ID;  //the ID of the transaction
    std::vector<std::string> approvedTx; //transactions approved by this transaction
};