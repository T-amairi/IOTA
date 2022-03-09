#include "MaliciousModule.h"

bool MaliciousModule::ifLaunchAttack() const
{
    SubmoduleIterator iter(getParentModule());
    iter++;

    for(; !iter.end(); iter++)
    {
        cModule *submodule = *iter;

        if(submodule->getId() != getId())
        {
            auto module = check_and_cast<AbstractModule*>(submodule);

            if(module->getTxCount() < module->getTxLimit() * attackStage)
            {
                return false;
            }
        }
    }

    return true;
}

bool MaliciousModule::ifModulesFinished() const
{
    SubmoduleIterator iter(getParentModule());
    iter++;

    for(; !iter.end(); iter++)
    {
        cModule *submodule = *iter;

        if(submodule->getId() != getId())
        {
            auto module = check_and_cast<AbstractModule*>(submodule);

            if(module->getTxCount() >= module->getTxLimit())
            {
                return true;
            }
        }
    }

    return false;
}

void MaliciousModule::setTxLimitModules() 
{
    SubmoduleIterator iter(getParentModule());
    iter++;

    for(; !iter.end(); iter++)
    {
        cModule *submodule = *iter;

        if(submodule->getId() != getId())
        {
            auto module = check_and_cast<AbstractModule*>(submodule);
            module->setTxLimit(module->getTxLimit() + module->getTxCount());
        }
    }
}

void MaliciousModule::printConflictTx()
{
    std::fstream file1;
    std::string path = "./data/tracking/NotLegit" + ID + ".csv";
    remove(path.c_str());
    file1.open(path,std::ios::app);

    path = "./data/tracking/Legit" + ID + ".csv";
    remove(path.c_str());
    std::fstream file2;
    file2.open(path,std::ios::app);

    for(auto DoubleSpendTx : myDoubleSpendTx)
    {   
        auto legitID = DoubleSpendTx->ID;
        legitID.erase(legitID.begin());

        for(auto tx : myTangle)
        {    
            if(DoubleSpendTx->ID != tx->ID)
            {
                if(isApp(tx,DoubleSpendTx->ID)) file1 << DoubleSpendTx->ID << ";" << tx->ID << std::endl;
                if(isApp(tx,legitID)) file2 << legitID << ";" << tx->ID << std::endl;
            }
        }
    }

    file1.close();
    file2.close();
}

void MaliciousModule::printBranchSize(bool ifDeleteFile)
{
    std::fstream file;
    std::string path = "./data/tracking/BranchSize" + ID + ".csv";
    if(ifDeleteFile) remove(path.c_str());
    file.open(path,std::ios::app);

    if(myBranch1.empty())
    {
        file << rateMB << ";" << "fail" << std::endl;
    }

    else
    {
        auto txRoot = myBranch1[0]->approvedTx[0];
        int count = 0;

        for(auto tx : myTangle)
        {
            if(txRoot->issuedTime <= tx->issuedTime)
            {
                if(!isApp(tx,myBranch1[0]->ID) && !isApp(tx,myBranch2[0]->ID))
                {
                    count++;
                }
            }
        }

        file << rateMB <<";" << count << ";" << myBranch1.size() << ";" << myBranch2.size() << std::endl;
    }

    file.close();
}

void MaliciousModule::printDiffTxChain(bool ifDeleteFile)
{
    std::fstream file;
    std::string path = "./data/tracking/DiffTxChain" + ID + ".csv";
    if(ifDeleteFile) remove(path.c_str());
    file.open(path,std::ios::app);

    if(myDoubleSpendTx.size() == 0)
    {
        file << "fail" << std::endl;
    }

    else
    {
        double pcp = par("propComputingPower");
        file << pcp << ";";

        for(auto DoubleSpendTx : myDoubleSpendTx)
        {
            file << DoubleSpendTx->ID << ";";

            int c1 = 0;
            int c2 = 0;

            for(auto tx : myTangle)
            {
                if(tx->issuedTime >= DoubleSpendTx->issuedTime && tx->ID != DoubleSpendTx->ID)
                {
                    isApp(tx,DoubleSpendTx->ID) ? c1++ : c2++;
                }
            }

           file << c1 - c2 << std::endl;
        }
    }

    file.close();
}

Tx* MaliciousModule::getTargetTx() const
{
    VpTx cpyTx;

    for(const auto tx : myTangle)
    {
        if(tx->ID.find(ID) != std::string::npos && tx->approvedBy.size() > 2)
        {
            cpyTx.push_back(tx);
        }
    }

    std::sort(cpyTx.begin(), cpyTx.end(), [](const Tx* tx1, const Tx* tx2){return tx1->issuedTime > tx2->issuedTime;});
  
    if(cpyTx.empty())
    {
        return nullptr;
    }

    return cpyTx[0];
}

Tx* MaliciousModule::getRootTx(simtime_t issuedTimeTarget) 
{
    std::map<int,Tx*> candidateRoot;

    for(auto tx : myTangle)
    {
        if(!tx->isGenesisBlock && tx->approvedBy.size() > 2 && tx->issuedTime < issuedTimeTarget)
        {
            candidateRoot.insert({computeWeight(tx),tx});
        }
    }

    if(candidateRoot.empty())
    {
        return nullptr;
    }

    return candidateRoot.rbegin()->second;
}

void MaliciousModule::linkChain(Tx* tx1, Tx* tx2) const
{
    tx1->approvedTx.push_back(tx2);
    tx2->approvedBy.push_back(tx1);

    if(!tx2->isApproved)
    {
        tx2->approvedTime = simTime();
        tx2->isApproved = true;
    }
}

VpTx MaliciousModule::getParasiteChain(Tx* rootTx, std::string targetID, int chainLength, int tipsCount)
{
    auto rootChain = new Tx("-" + targetID);
    rootChain->issuedTime = simTime();
    myDoubleSpendTx.push_back(rootChain);
    myTangle.push_back(rootChain);

    linkChain(rootChain,rootTx);
    updateMyTips(rootTx->ID);

    VpTx TheChain = {rootChain};
    
    for(int i = 0; i < chainLength; i++)
    {
        auto txChain = createTx();

        if(i == 0)
        {
            linkChain(txChain,rootChain);
        }

        else
        {
            linkChain(txChain,TheChain.back());
        }

        TheChain.push_back(txChain);
        myTangle.push_back(txChain);
    }

    auto backChain = TheChain.back();

    for(int i = 0; i < tipsCount; i++)
    {
        auto tipChain = createTx();

        if(i == 0)
        {
            linkChain(tipChain,backChain);
        }

        else
        {
            tipChain->approvedTx.push_back(backChain);
            backChain->approvedBy.push_back(tipChain);
        }

        TheChain.push_back(tipChain);
        myTangle.push_back(tipChain);
        myTips.push_back(tipChain);
    }

    return TheChain;
}

void MaliciousModule::iniSPA()
{
    auto tx1 = createTx();
    auto tx2 = createTx();

    auto root1 = myBranch1[0];
    auto root2 = myBranch2[0];

    linkChain(tx1,root1);
    linkChain(tx2,root2);

    myBranch1.push_back(tx1);
    myBranch2.push_back(tx2);
}

void MaliciousModule::mergeAndBroadcast()
{
    auto rootTip = myTips.back();
    auto rootBr1 = myBranch1[0];
    auto rootBr2 = myBranch2[0];

    linkChain(rootBr1,rootTip);
    linkChain(rootBr2,rootTip);
    updateMyTips(rootTip->ID);

    std::vector<VpTx> myBranches = {myBranch1,myBranch2};
        
    for(auto branch : myBranches)
    {
        for(auto tx : branch)
        {
            myTangle.push_back(tx);

            if(!tx->isApproved) myTips.push_back(tx);

            broadcastTx(tx);
        }
    }

    myDoubleSpendTx.push_back(rootBr2);
}

void MaliciousModule::updateBranches(Tx* tip)
{
    for(const auto tx : tip->approvedTx)
    {
        auto it1 = std::find_if(myBranch1.rbegin(), myBranch1.rend(), [tx](const Tx* tip) {return tip->ID == tx->ID;});

        if(it1 != myBranch1.rend())
        {
            myBranch1.push_back(tip);
            return;
        }

        auto it2 = std::find_if(myBranch2.rbegin(), myBranch2.rend(), [tx](const Tx* tip) {return tip->ID == tx->ID;});

        if(it2 != myBranch2.rend())
        {
            myBranch2.push_back(tip);
            return;
        }
    }
}

void MaliciousModule::handleBalance()
{
    EV << "Checking the balance...\n";
    int sizeDiff = myBranch1.size() - myBranch2.size();
    countMB = std::abs(sizeDiff);

    if(sizeDiff != 0)
    {
        EV << "The weight of the two branches is not the same : maintaining...\n";

        if(sizeDiff > 0) whichBranch = 2;
        else whichBranch = 1;
    
        for(int i = 0; i < std::abs(sizeDiff); i++)
        {
            scheduleAt(simTime() + (i+1)*rateMB, msgMaintainBalance->dup());
        }
    }

    else
    {
        EV << "The weight of the two branches is the same\n";
    }
}

Tx* MaliciousModule::maintainBalance()
{
    VpTx chosenBranch;

    if(whichBranch == 2) chosenBranch = myBranch2;
    else chosenBranch = myBranch1;
    
    int countTips = 0;

    for(auto tx : chosenBranch)
    {
        if(!tx->isApproved) countTips++;
        if(countTips > totalModules - 1) break;
    }
    
    bool ifAppTx = countTips <= totalModules - 1;
    VpTx candidatesTx;

    for(auto tx : chosenBranch)
    {
        if(tx->isApproved && ifAppTx) candidatesTx.push_back(tx);
        else if(!tx->isApproved && !ifAppTx) candidatesTx.push_back(tx);
    }

    int randomIdx = intuniform(0,candidatesTx.size() - 1);
    auto txToApp = candidatesTx[randomIdx];
    candidatesTx.erase(candidatesTx.begin() + randomIdx);

    auto newTip = createTx();
    linkChain(newTip,txToApp);
    if(!ifAppTx) updateMyTips(txToApp->ID);

    if(!candidatesTx.empty())
    {
        randomIdx = intuniform(0,candidatesTx.size() - 1);
        txToApp = candidatesTx[randomIdx];

        linkChain(newTip,txToApp);
        if(!ifAppTx) updateMyTips(txToApp->ID);
    }

    if(whichBranch == 2) myBranch2.push_back(newTip);
    else myBranch1.push_back(newTip);

    myTangle.push_back(newTip);
    myTips.push_back(newTip);

    return newTip;
}

void MaliciousModule::caseATTACK()
{
    if(par("PCattack"))
    {
        scheduleAt(simTime(), msgParasiteChain);
    }

    else if(par("SPattack"))
    {
        auto tx1 = createTx();
        auto tx2 = new Tx("-" + tx1->ID);
        tx2->issuedTime = simTime();

        myBranch1.push_back(tx1);
        myBranch2.push_back(tx2);

        scheduleAt(simTime() + 2*rateMB, msgSplitting);
    }
}

void MaliciousModule::casePCA()
{
    auto targetTx = getTargetTx();

    if(targetTx == nullptr)
    {
        EV << "Can not find a legit transaction for the chain, retrying later\n";
        scheduleAt(simTime() + exponential(rateMean), msgParasiteChain);
        return;
    }

    auto rootTx = getRootTx(targetTx->issuedTime);

    if(rootTx == nullptr)
    {
        EV << "Can not find a legit transaction for the chain, retrying later\n";
        scheduleAt(simTime() + exponential(rateMean), msgParasiteChain);
        return;
    }

    auto T_diff = targetTx->issuedTime - rootTx->issuedTime;
    double PCP = par("propComputingPower");
    double PCT = par("propChainTips");
    PCP = PCP * rateMean.dbl();
    int weightChain = int(T_diff.dbl()/PCP);

    EV << "Launching a Parasite Chain Attack !\n";
    auto theChain = getParasiteChain(rootTx,targetTx->ID,weightChain * PCT,weightChain * (1 - PCT));

    for(const auto tx : theChain)
    {
        broadcastTx(tx);
    }
}

void MaliciousModule::caseSPA()
{
    if(ifModulesFinished())
    {
        EV << "Splitting attack failed to launch.\n";

        std::vector<VpTx> myBranches = {myBranch1,myBranch2};
        
        for(auto branch : myBranches)
        {
            for(auto tx : branch)
            {
                delete tx;
            }
        }

        myBranch1.clear();
        myBranch2.clear();
    }

    else if(myBranch1.size() + myBranch2.size() >= myTips.size() * sizeBrancheProp)
    {
        EV << "Launching a splitting attack.\n";

        mergeAndBroadcast();
        setTxLimitModules();

        countMB = 0;
        cachedRequest = false;
        ifSimFinished = false;
        msgMaintainBalance = new cMessage("Maintaining the balance between the two branches",MB);
    }

    else
    {
        EV << "Preparing the two branches in offline...\n";

        iniSPA();
        scheduleAt(simTime() + 2*rateMB, msgSplitting);
    }
}

void MaliciousModule::caseMB(cMessage* msg)
{
    delete msg;

    ifSimFinished = ifModulesFinished();

    if(ifSimFinished) return;

    if(countMB != 0)
    {
        EV << "Updated the balance of the branches\n";
        
        auto newTx = maintainBalance();
        broadcastTx(newTx);
        countMB--;
    }

    if(cachedRequest && countMB == 0)
    {
        cachedRequest = false;
        handleBalance();
    }
}

void MaliciousModule::caseISSUE()
{
    if(ifLaunchAttack())
    {   
        scheduleAt(simTime(), msgAttack);
        return;
    }

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

void MaliciousModule::casePOW(cMessage* msg)
{
    auto chosenTips = (VpTx*) msg->getContextPointer();
    auto newTx = attachTx(simTime(),*chosenTips);

    EV << "Pow time finished for " << newTx->ID << ", sending it to all nodes\n";

    broadcastTx(newTx);
    scheduleAt(simTime() + exponential(rateMean), msgIssue);
}

void MaliciousModule::caseUPDATE(cMessage* msg)
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

    auto newTip = updateTangle(data);

    if(msgMaintainBalance)
    {
        updateBranches(newTip);
        
        if(!ifSimFinished)
        {
            if(countMB == 0) handleBalance();
            else cachedRequest = true;
        }
    } 

    spreadTx(msg->getSenderModule(),data);

    delete data;
    delete msg; 
}

void MaliciousModule::initialize()
{
    _initialize();

    attackStage = par("attackStage");
    msgAttack = new cMessage("Initiating an attack",ATTACK);

    if(par("PCattack"))
    {
        msgParasiteChain = new cMessage("Initiating a parasite chain attack",PCA);
    }

    else if(par("SPattack"))
    {   
        sizeBrancheProp = par("sizeBrancheProp");
        double propRateMB = par("propRateMB");
        rateMB = rateMean * propRateMB;
        totalModules = int(getParentModule()->par("nbMaliciousNode")) + int(getParentModule()->par("nbHonestNode"));
        msgSplitting = new cMessage("Initiating a splitting attack",SPA);
    }

    EV << "Initialization complete\n";
    scheduleAt(simTime() + exponential(rateMean), msgIssue);
}

void MaliciousModule::handleMessage(cMessage* msg)
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

        case MessageType::ATTACK:
            caseATTACK();
            break;

        case MessageType::PCA:
            casePCA();
            break;
        
        case MessageType::SPA:
            caseSPA();
            break;

        case MessageType::MB:
            caseMB(msg);

        default:
            break;
    }
}

void MaliciousModule::finish()
{
    _finish(static_cast<bool>(par("exportTangle")),std::make_pair(static_cast<bool>(par("exportTipsNumber")),static_cast<bool>(par("wipeLogTipsNumber"))));

    delete msgAttack;

    if((msgParasiteChain || msgSplitting) && static_cast<bool>(par("exportConflictTx")))
    {
        printConflictTx();
    }

    if(msgParasiteChain)
    {
        if(static_cast<bool>(par("exportDiffTxChain"))) printDiffTxChain(static_cast<bool>(par("wipeLogDiffTxChain")));
        delete msgParasiteChain;
    }

    else if(msgSplitting)
    {
        if(static_cast<bool>(par("exportBranchSize"))) printBranchSize(static_cast<bool>(par("wipeLogBranchSize")));
        delete msgSplitting;
        if(msgMaintainBalance) delete msgMaintainBalance;
    }
}
