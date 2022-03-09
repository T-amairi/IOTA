#include "HonestModule.h"

void HonestModule::caseISSUE()
{
    if(txCount <= txLimit)
    {
        auto chosenTips = getTipsTSA();

        if(chosenTips.empty())
        {
            EV << "The TSA did not give legit tips to approve: attempting again\n";
            scheduleAt(simTime() + exponential(rateMean), msgIssue);
            return;
        }

        EV << "Chosen Tips: ";

        for(const auto tip : chosenTips) EV << tip->ID << " "; EV << "\n Pow time:"  << chosenTips.size() * powTime << "\n";

        msgPoW->setContextPointer(&chosenTips);

        if(powTime == 0.0)
        {
            casePOW(msgPoW);
        }

        else
        {
            scheduleAt(simTime() + chosenTips.size() * powTime, msgPoW);
        }
    }

    else
    {
        EV << "Number of transactions reached: stopping issuing\n";
    }
}

void HonestModule::casePOW(cMessage* msg)
{
    auto chosenTips = (VpTx*) msg->getContextPointer();
    auto newTx = attachTx(simTime(),*chosenTips);

    EV << "Pow time finished for " << newTx->ID << ", sending it to all nodes\n";
    
    broadcastTx(newTx);
    scheduleAt(simTime() + exponential(rateMean), msgIssue);
}

void HonestModule::caseUPDATE(cMessage* msg)
{
    auto data =  (dataUpdate*) msg->getContextPointer();

    if(isPresent(data->ID))
    {
        EV << "Transaction " << data->ID << " is already present\n";
        delete data;
        delete msg;
        return;
    }

    EV << "Received a new transaction " << data->ID << "\n";

    updateTangle(data);
    updateMyBuffer();
    spreadTx(msg->getSenderModule(),data);
    
    delete data;
    delete msg; 
}

void HonestModule::initialize()
{
    _initialize();

    EV << "Initialization complete\n";
    scheduleAt(simTime() + exponential(rateMean), msgIssue);
}

void HonestModule::handleMessage(cMessage* msg)
{
    switch (msg->getKind())
    {
        case MessageType::ISSUE:
            caseISSUE();
            break;

        case MessageType::POW:
            casePOW(msg);
            break;

        case MessageType::UPDATE:
            caseUPDATE(msg);
            break;

        default:
            break;
    }
}

void HonestModule::finish()
{
    _finish(static_cast<bool>(par("exportTangle")),std::make_pair(static_cast<bool>(par("exportTipsNumber")),static_cast<bool>(par("wipeLogTipsNumber"))));
}
