//includes
#include <stdio.h>
#include <string>
#include <omnetpp.h>
#include <fstream>
#include <sstream>
#include "iota.h"

using namespace omnetpp;

enum MessageType{UPDATE, ISSUE, STOP};

std::ofstream LOG_SIM;

class NodeModule : public cSimpleModule
{
    public:
        std::string NodeId;
        std::string ShortId;
        int TrCount;
        int stopCount;
        int trLimit;
        double prob;
        int NeighborsNumber;
        Node self;
        RNG rng;
        simtime_t powTime;

    protected:
        virtual void initialize() override;
        virtual void handleMessage(cMessage * msg) override;
};

Define_Module(NodeModule);

void NodeModule::initialize()
{
    std::string file = "./data/Tracking/logNodeModule[" + std::to_string(getId() - 2 ) + "].txt";
    LOG_SIM.open(file.c_str(),std::ios::app);

    LOG_SIM << "START" << std::endl;

    TrCount = 0;
    stopCount = 0;
    NeighborsNumber = getParentModule()->par("NodeNumber");
    powTime = par("powTime");
    prob = par("prob");
    NodeId = getFullName();
    ShortId = "[" + std::to_string(getId()) + "]";
    trLimit = getParentModule()->par("transactionLimit");

    self.ID = ShortId;
    self.genesisBlock = self.createGenBlock();
    self.myTangle.push_back(self.genesisBlock);
    self.setRNGPtr(&rng) ;

    LOG_SIM << "Initialization complete, starting first try to issue a transaction" << std::endl;

    cMessage * msg = new cMessage("First try to issue a transaction",ISSUE);
    scheduleAt(simTime() + par("trgenRate"), msg);

    LOG_SIM << "END" << std::endl;
}

void NodeModule::handleMessage(cMessage * msg)
{
    if(msg->getKind() == ISSUE)
    {
        TrCount++;

        bool test = self.ifIssue(prob);
        int sizeTangle = static_cast<int>(self.myTangle.size());

        EV << "Trying to issue a new transaction" << std::endl;
        EV << "ifIssue result : " << test;

        LOG_SIM << "Trying to issue a new transaction" << std::endl;
        LOG_SIM << "ifIssue result : " << test << std::endl;

        if(test && sizeTangle < trLimit)
        {
            LOG_SIM << "TSA procedure, selecting tips to approve ";

            delete msg;
            pTr_S new_tr;
            int tipsNb = 0;
            std::map<int,pTr_S> tipsCopy = self.giveTips();
            std::string trId = ShortId + std::to_string(TrCount);

            EV << "Selecting tips to approve " << trId;
            LOG_SIM << trId << std::endl;

            if(strcmp(par("TSA"),"IOTA") == 0)
            {
               VpTr_S chosenTips = self.IOTA(par("alpha"),tipsCopy,simTime(),par("W"),par("N"));
               tipsNb = static_cast<int>(chosenTips.size());
               new_tr = self.attach(self,trId,simTime(),chosenTips);
            }

            if(strcmp(par("TSA"),"GIOTA") == 0)
            {
               VpTr_S chosenTips = self.GIOTA(par("alpha"),tipsCopy,simTime(),par("W"),par("N"));
               tipsNb = static_cast<int>(chosenTips.size());
               new_tr = self.attach(self,trId,simTime(),chosenTips);
            }

            if(strcmp(par("TSA"),"EIOTA") == 0)
            {
                VpTr_S chosenTips = self.EIOTA(par("p1"),par("p2"),tipsCopy,simTime(),par("W"),par("N"));
                tipsNb = static_cast<int>(chosenTips.size());
                new_tr = self.attach(self,trId,simTime(),chosenTips);
            }

            cMessage * SendTangle = new cMessage("UpdatedTangle",UPDATE);
            SendTangle->setContextPointer(&new_tr);

            EV << "Pow time = " << tipsNb*powTime;
            EV << "Sending the new transaction to all nodes";

            LOG_SIM << "Pow time = " << tipsNb*powTime << std::endl;
            LOG_SIM << "Sending the new transaction to all nodes" << std::endl;

            for(int i = 0; i < NeighborsNumber; i++)
            {
                sendDelayed(SendTangle,tipsNb*powTime,"NodeConnect$o", i);
            }

            cMessage * new_issue = new cMessage("Try to issue a transaction",ISSUE);
            scheduleAt(simTime() + par("trgenRate"), new_issue);
        }

        else if(!test && sizeTangle < trLimit)
        {
            LOG_SIM << "Prob not higher than test, retrying to issue again" << std::endl;

            cMessage * new_issue = new cMessage("Try to issue a transaction",ISSUE);
            scheduleAt(simTime() + par("trgenRate"), new_issue);
        }

        else if(sizeTangle >= trLimit)
        {
            cMessage * Stop = new cMessage("Stop Emulation : transaction limit reach",STOP);

            EV << "I have reached trLimit";
            LOG_SIM << "I have reached trLimit" << std::endl;

            if(!self.ifDeleted)
           {
                EV << "Deleting my local Tangle";
                LOG_SIM << "Deleting my local Tangle" << std::endl;
                self.DeleteTangle(self, self.myTangle);
           }

            EV << "Sending stop message to all nodes";
            LOG_SIM << "Sending stop message to all nodes" << std::endl;

            for(int i = 0; i < NeighborsNumber; i++)
            {
                send(Stop,"NodeConnect$o", i);
            }
        }
    }

    else if(msg->getKind() == STOP)
    {
        delete msg;
        stopCount++;

        LOG_SIM << "Time to end simulation" << std::endl;
        EV << "Time to end simulation";

        if(!self.ifDeleted)
        {
            EV << "Deleting my local Tangle";
            LOG_SIM << "Deleting my local Tangle" << std::endl;
            self.DeleteTangle(self, self.myTangle);
        }

        cMessage * Stop = new cMessage("Stop Emulation : transaction limit reached",STOP);

        EV << "Sending stop message to all nodes";
        LOG_SIM << "Sending stop message to all nodes" << std::endl;

        for(int i = 0; i < NeighborsNumber; i++)
        {
            send(Stop,"NodeConnect$o", i);
        }

        if(stopCount >= NeighborsNumber - 1)
        {
            EV << "All nodes have free their respective Tangles, ending simulation now";
            LOG_SIM << "All nodes have free their respective Tangles, ending simulation now" << std::endl;
            endSimulation();
        }
    }

    else if(msg->getKind() == UPDATE)
    {
        delete msg;
        EV << "Updating Tangle";
        LOG_SIM << "Updating Tangle" << std::endl;
        pTr_S* uptatedTangle = (pTr_S*) msg->getContextPointer();
        self.updateTangle(*uptatedTangle,simTime());
    }
}
