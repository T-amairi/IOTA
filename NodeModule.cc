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

Define_Module(NodeModule);

void NodeModule::initialize()
{
    std::string file = "./data/Tracking/logNodeModule[" + std::to_string(getId() - 2) + "].txt";
    LOG_SIM.open(file.c_str(),std::ios::app);

    TrCount = 0;
    stopCount = 0;
    NeighborsNumber = getParentModule()->par("NodeNumber");
    NeighborsNumber--;
    powTime = par("powTime");
    prob = par("prob");
    ShortId = "[" + std::to_string(getId() - 2) + "]";
    trLimit = getParentModule()->par("transactionLimit");

    self.ID = ShortId;
    self.genesisBlock = self.createGenBlock();
    self.myTangle.push_back(self.genesisBlock);
    self.myTips.insert({"Genesis",self.genesisBlock});

    LOG_SIM << simTime() << " Initialization complete, starting first try to issue a transaction" << std::endl;
    EV << "Initialization complete, starting first try to issue a transaction" << std::endl;

    cMessage * msg = new cMessage("First try to issue a transaction",ISSUE);
    scheduleAt(simTime() + par("trgenRate"), msg);

    LOG_SIM.close();
}

void NodeModule::handleMessage(cMessage * msg)
{
    std::string file = "./data/Tracking/logNodeModule[" + std::to_string(getId() - 2 ) + "].txt";
    LOG_SIM.open(file.c_str(),std::ios::app);

    if(msg->getKind() == ISSUE)
    {
        delete msg;
        bool test = self.ifIssue(prob);
        int sizeTangle = static_cast<int>(self.myTangle.size());

        EV << "Trying to issue a new transaction" << std::endl;
        EV << "ifIssue result : " << test << std::endl;

        LOG_SIM << simTime() << " Trying to issue a new transaction" << std::endl;
        LOG_SIM << simTime() << " ifIssue result : " << test << std::endl;

        if(test && sizeTangle < trLimit)
        {
            TrCount++;
            pTr_S new_tr;
            int tipsNb = 0;
            std::string trId = ShortId + std::to_string(TrCount);

            EV << "TSA procedure for " << trId<< std::endl;
            LOG_SIM << simTime() << " TSA procedure for " << trId << std::endl;

            if(strcmp(par("TSA"),"IOTA") == 0)
            {
               std::map<std::string, pTr_S> tipsCopy = self.giveTips();
               LOG_SIM << " Tips number : " << tipsCopy.size() << std::endl;
               VpTr_S chosenTips = self.IOTA(par("alpha"),tipsCopy,simTime(),par("W"),par("N"));
               tipsNb = static_cast<int>(chosenTips.size());
               new_tr = self.attach(trId,simTime(),chosenTips);
            }

            if(strcmp(par("TSA"),"GIOTA") == 0)
            {
               std::map<std::string, pTr_S> tipsCopy = self.giveTips();
               VpTr_S chosenTips = self.GIOTA(par("alpha"),tipsCopy,simTime(),par("W"),par("N"));
               tipsNb = static_cast<int>(chosenTips.size());
               new_tr = self.attach(trId,simTime(),chosenTips);
            }

            if(strcmp(par("TSA"),"EIOTA") == 0)
            {
                std::map<std::string, pTr_S> tipsCopy = self.giveTips();
                VpTr_S chosenTips = self.EIOTA(par("p1"),par("p2"),tipsCopy,simTime(),par("W"),par("N"));
                tipsNb = static_cast<int>(chosenTips.size());
                new_tr = self.attach(trId,simTime(),chosenTips);
            }

            cMessage * SendTangle = new cMessage("UpdatedTangle",UPDATE);

            MsgUpdate * Msg = new MsgUpdate;
            Msg->ID = new_tr->ID;
            Msg->issuedBy = new_tr->issuedBy;
            Msg->issuedTime = new_tr->issuedTime;

            for(auto approvedTips : new_tr->S_approved)
            {
                Msg->S_approved.push_back(approvedTips->ID);
            }

            SendTangle->setContextPointer(Msg);

            EV << "Pow time = " << tipsNb*powTime<< std::endl;
            EV << "Sending the new transaction to all nodes"<< std::endl;

            LOG_SIM << simTime() << " Pow time = " << tipsNb*powTime << std::endl;
            LOG_SIM << simTime() << " Sending the new transaction to all nodes" << std::endl;

            for(int i = 0; i < NeighborsNumber; i++)
            {
                sendDelayed(SendTangle,tipsNb*powTime,"NodeOut", i);
            }

            cMessage * new_issue = new cMessage("Try to issue a transaction",ISSUE);
            scheduleAt(simTime() + par("trgenRate"), new_issue);
        }

        else if(!test && sizeTangle < trLimit)
        {
            LOG_SIM << simTime() << " Prob not higher than test, retrying to issue again" << std::endl;
            EV << "Prob not higher than test, retrying to issue again" << std::endl;

            cMessage * new_issue = new cMessage("Try to issue a transaction",ISSUE);
            scheduleAt(simTime() + par("trgenRate"), new_issue);
        }

        else if(sizeTangle >= trLimit)
        {
            cMessage * Stop = new cMessage("Stop Emulation : transaction limit reach",STOP);

            EV << "I have reached trLimit"<< std::endl;
            LOG_SIM << simTime() << " I have reached trLimit" << std::endl;

            if(!self.ifDeleted)
           {
                EV << "Deleting my local Tangle"<< std::endl;
                LOG_SIM << simTime() << " Deleting my local Tangle" << std::endl;
                self.printTangle(self.myTangle, getId() - 2);
                self.printTipsLeft(static_cast<int>(self.myTips.size()), getId() - 2);
                self.DeleteTangle(self, self.myTangle);
           }

            EV << "Sending stop message to all nodes" << std::endl;
            LOG_SIM << simTime() << " Sending stop message to all nodes" << std::endl;

            for(int i = 0; i < NeighborsNumber; i++)
            {
                send(Stop,"NodeOut", i);
            }
        }

        LOG_SIM.close();
    }

    else if(msg->getKind() == STOP)
    {
        delete msg;
        stopCount++;

        LOG_SIM << simTime() << " Time to end simulation" << std::endl;
        EV << "Time to end simulation"<< std::endl;

        if(!self.ifDeleted)
        {
            EV << "Deleting my local Tangle"<< std::endl;
            LOG_SIM << simTime() << " Deleting my local Tangle" << std::endl;
            self.printTangle(self.myTangle, getId() - 2);
            self.printTipsLeft(static_cast<int>(self.myTips.size()), getId() - 2);
            self.DeleteTangle(self, self.myTangle);
        }

        cMessage * Stop = new cMessage("Stop Emulation : transaction limit reached",STOP);

        EV << "Sending stop message to all nodes"<< std::endl;
        LOG_SIM << simTime() << " Sending stop message to all nodes" << std::endl;

        for(int i = 0; i < NeighborsNumber; i++)
        {
            send(Stop,"NodeOut", i);
        }

        if(stopCount >= NeighborsNumber - 1)
        {
            EV << "All nodes have free their respective Tangles, ending simulation now"<< std::endl;
            LOG_SIM << simTime() << " All nodes have free their respective Tangles, ending simulation now" << std::endl;
            LOG_SIM.close();
            endSimulation();
        }
    }

    else if(msg->getKind() == UPDATE)
    {
        EV << "Updating Tangle"<< std::endl;
        LOG_SIM << simTime() << " Updating Tangle" << std::endl;
        MsgUpdate* Msg = (MsgUpdate*) msg->getContextPointer();
        self.updateTangle(Msg,simTime());
        delete Msg;
        delete msg;
        LOG_SIM.close();
    }
}
