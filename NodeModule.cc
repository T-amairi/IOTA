//includes
#include "iota.h"

enum MessageType{ISSUE,UPDATE};

std::ofstream LOG_SIM;

Define_Module(NodeModule);

int NodeModule::rangeRandom(int min, int max)
{
    return intuniform(min,max);
}

bool NodeModule::ifIssue(double prob)
{
    double test = uniform(0.0,1.0);

    if(test <= prob)
    {
        return true;
    }

    return false;
}

pTr_S NodeModule::createSite(std::string ID)
{
    pTr_S newTx = new Site;

    newTx->ID =  ID;
    newTx->issuedBy = this;
    newTx->issuedTime = simTime();

    return newTx;
}

pTr_S NodeModule::createGenBlock()
{
    pTr_S GenBlock = createSite("Genesis");
    GenBlock->isGenesisBlock = true;

    return GenBlock;
}

void NodeModule::DeleteTangle()
{
    for(auto site : myTangle)
    {
            delete site;
    }
}

void NodeModule::printTangle()
{
    std::fstream file;
    std::string path = "./data/Tracking/TrackerTangle" + ID + ".txt";
    file.open(path,std::ios::app);

    simtime_t sec = simTime();

    for(long unsigned int i = 0; i < myTangle.size(); i++)
    {
       file << sec << ";";
       file << myTangle[i]->ID << ";";

        for(long unsigned int j = 0; j < myTangle[i]->approvedBy.size(); j++)
        {
            auto temp = myTangle[i]->approvedBy[j];

            if(j == myTangle[i]->approvedBy.size() - 1)
            {
                file << temp->ID;
            }

            else
            {
                file << temp->ID << ",";
            }
        }

        file << std::endl;
    }

    file.close();
}

void NodeModule::printTipsLeft()
{
    std::fstream file;
    std::string path = "./data/Tracking/NumberTips" + ID + ".txt";
    file.open(path,std::ios::app);

    file << myTips.size() << std::endl;

    file.close();
}

void NodeModule::debug()
{
    std::fstream file;
    std::string path = "./data/Tracking/debug" + ID + ".txt";
    file.open(path,std::ios::app);
    file << simTime() << " HERE" << std::endl;
    file.close();
}

std::map<std::string,pTr_S> NodeModule::giveTips()
{
     return myTips;
}

int NodeModule::_computeWeight(VpTr_S& visited, pTr_S& current, simtime_t timeStamp)
{
    if(timeStamp < current->issuedTime)
    {
        visited.push_back(current);
        current->isVisited = true;
        return 0;
    }

    if(current->approvedBy.size() == 0)
    {
        visited.push_back(current);
        current->isVisited = true;
        return 0;
    }

    visited.push_back(current);

    current->isVisited = true;
    int weight = 0;

    for(int i = 0; i < static_cast<int>(current->approvedBy.size()); ++i)
    {
        if(!current->approvedBy.at(i)->isVisited)
        {
                weight += 1 + _computeWeight(visited, current->approvedBy.at(i), timeStamp);
        }
    }

    return weight;
}

int NodeModule::ComputeWeight(pTr_S tr, simtime_t timeStamp)
{
    VpTr_S visited;
    int weight = _computeWeight(visited, tr, timeStamp);

    for(int i = 0; i < static_cast<int>(visited.size()); ++i)
    {
        for(int j = 0; j < static_cast<int>(visited.at(i)->approvedBy.size()); ++j)
        {
            visited.at(i)->approvedBy.at(j)->isVisited = false;
        }

    }

    tr->isVisited = false;
    return weight + 1;
}

pTr_S NodeModule::getWalkStart(std::map<std::string,pTr_S>& tips, int backTrackDist)
{
    int iterAdvances = rangeRandom(0,tips.size() - 1);
    auto beginIter = tips.begin();

    if(tips.size() > 1)
    {
        std::advance(beginIter,iterAdvances);
    }

    pTr_S current = beginIter->second;

    int count = backTrackDist;
    int approvesIndex;

    while(!current->isGenesisBlock && count > 0)
    {
        approvesIndex = rangeRandom(0,current->S_approved.size() - 1);

        if(current->S_approved.size() == 0)
        {
            printTangle();
        }
        current = current->S_approved.at(approvesIndex);
        --count;
    }

    return current;
}

bool NodeModule::isRelativeTip(pTr_S& toCheck, std::map<std::string, pTr_S>& tips)
{
    std::map<std::string,pTr_S>::iterator it;

    for(it = tips.begin(); it != tips.end(); it++)
    {
        if(it->first.compare(toCheck->ID) == 0)
        {
            break;
        }
    }

    if(it == tips.end())
    {
        return false;
    }

    return true;
}

void NodeModule::filterView(VpTr_S& view, simtime_t timeStamp)
{
    std::vector<int> removeIndexes;

    for(int i = 0; i < static_cast<int>(view.size()); ++i)
    {
        if(view.at(i)->issuedTime > timeStamp)
        {
            removeIndexes.push_back(i);
        }
    }

    if(removeIndexes.size() > 0)
    {
        for(int i = removeIndexes.size() - 1; i > -1; --i)
        {
            view.erase(view.begin() + removeIndexes.at(i));
        }
    }
}

void NodeModule::ReconcileTips(const VpTr_S& removeTips, std::map<std::string,pTr_S>& myTips)
{
    std::map<std::string,pTr_S>::iterator it;

    for(auto& tipSelected : removeTips)
    {
       for(it = myTips.begin(); it != myTips.end();)
       {
           if(tipSelected->ID.compare(it->first) == 0)
           {
               it = myTips.erase(it);

           }

           else
           {
               ++it;
           }
       }
    }
}

pTr_S NodeModule::attach(std::string ID, simtime_t attachTime, VpTr_S& chosen)
{
     pTr_S new_tips = createSite(ID);

     for (auto& tipSelected : chosen)
     {
         tipSelected->approvedBy.push_back(new_tips);

         if(!(tipSelected->isApproved))
         {
             tipSelected->approvedTime = attachTime;
             tipSelected->isApproved = true;
         }

     }

     new_tips->S_approved = chosen;
     ReconcileTips(chosen,myTips);

     myTangle.push_back(new_tips);
     myTips.insert({new_tips->ID,new_tips});
     return new_tips;
}

//TODO: case alpha = 0.0 (dont compute weight)
pTr_S NodeModule::RandomWalk(pTr_S start, double alphaVal, std::map<std::string, pTr_S>& tips, simtime_t timeStamp, int &walk_time)
{
    int walkCounts = 0;
    pTr_S current = start;

    while(!isRelativeTip(current,tips))
    {
        walkCounts++;

        int start_weight = ComputeWeight(current,timeStamp);
        VpTr_S currentView = current->approvedBy;
        std::vector<int> sitesWeight;
        std::vector<std::pair<int,double>> sitesProb;
        filterView(currentView,timeStamp);

        if(currentView.size() == 0)
        {
            break;
        }

        if(currentView.size() == 1)
        {
            current = currentView.front();
        }

        else
        {
            double sum_exp = 0.0;
            int weight;

            for(int j = 0; j < static_cast<int>(currentView.size()); j++)
            {
                weight = ComputeWeight(currentView[j],timeStamp);
                sum_exp = sum_exp + double(exp(double(-alphaVal*(start_weight - weight))));
                sitesWeight.push_back(weight);
            }

            double prob;

            for(int j = 0; j < static_cast<int>(sitesWeight.size()); j++)
            {
               prob = double(exp(double(-alphaVal*(start_weight - sitesWeight[j]))));
               prob = prob/sum_exp;
               sitesProb.push_back(std::make_pair(j,prob));
            }

            std::sort(sitesProb.begin(), sitesProb.end(),[](const std::pair<int,double> &a, const std::pair<int,double> &b){
            return a.second < b.second;});
            int nextCurrentIndex = 0;
            double probWalkChoice =  uniform(0.0,1.0);

            for(int j = 0; j < static_cast<int>(sitesProb.size()) - 1; j++)
            {
                if(j == 0)
                {
                    if(probWalkChoice < sitesProb[j].second)
                    {
                        nextCurrentIndex = sitesProb[j].first;
                        break;
                    }
                }

                else
                {
                    if(probWalkChoice < sitesProb[j-1].second + sitesProb[j].second)
                    {
                        nextCurrentIndex = sitesProb[j].first;
                        break;
                    }
                }
            }

            current = currentView[nextCurrentIndex];
        }
    }

    walk_time = walkCounts;
    current->walkBacktracks = walkCounts;

    return current;
}

VpTr_S NodeModule::IOTA(double alphaVal, std::map<std::string, pTr_S>& tips, simtime_t timeStamp, int W, int N)
{
    VpTr_S start_sites;
    pTr_S temp_site;
    int backTD;

    for(int i = 0; i < N; i++)
    {
        backTD = rangeRandom(W,2*W);
        temp_site = getWalkStart(tips,backTD);
        start_sites.push_back(temp_site);
    }

    VpTr_S selected_tips;
    std::vector<std::pair<int,int>> walk_total;
    int walk_time;

    for(int i = 0; i < N; i++)
    {
        selected_tips.push_back(RandomWalk(start_sites[i],alphaVal,tips,timeStamp,walk_time));
        walk_total.push_back(std::make_pair(i,walk_time));
    }

    std::sort(walk_total.begin(), walk_total.end(),[](const std::pair<int,int> &a, const std::pair<int,int> &b){
    return a.second < b.second;});

    VpTr_S tipstoApprove;
    int Index_first_tips = walk_total[0].first;
    tipstoApprove.push_back(selected_tips[Index_first_tips]);

   for(long unsigned int j = 1; j < walk_total.size(); j++)
   {
       Index_first_tips = walk_total[j].first;

       if(selected_tips[Index_first_tips]->ID != tipstoApprove[0]->ID)
       {
           tipstoApprove.push_back(selected_tips[Index_first_tips]);
           break;
       }
   }

    return tipstoApprove;
}

VpTr_S NodeModule::GIOTA(double alphaVal, std::map<std::string, pTr_S>& tips, simtime_t timeStamp, int W, int N)
{
   auto copyTips = tips;
   VpTr_S chosenTips = IOTA(alphaVal,tips,timeStamp,W,N);
   VpTr_S filterTips;

   for(auto it = copyTips.begin(); it != copyTips.end(); ++it)
   {
       auto tip = it->second;

       for(int j = 0; j < static_cast<int>(chosenTips.size()); j++)
       {
           if(chosenTips[j]->ID.compare(tip->ID) != 0)
           {
               filterTips.push_back(tip);
               break;
           }
       }
   }

   if(!filterTips.size())
   {
       return chosenTips;
   }

   auto idx = rangeRandom(0,filterTips.size() - 1);
   auto LeftTips = filterTips[idx];

   chosenTips.push_back(LeftTips);
   return chosenTips;
}

VpTr_S NodeModule::EIOTA(double p1, double p2, std::map<std::string, pTr_S>& tips, simtime_t timeStamp, int W, int N)
{
    double lowAlpha = 0.1;
    double highAlpha = 5.0;
    auto r = uniform(0.0,1.0);
    VpTr_S chosenTips;

    if(r < p1)
    {
        chosenTips = IOTA(0.0,tips,timeStamp,W,N);
    }

    else if(p1 <= r && r < p2)
    {
       chosenTips = IOTA(lowAlpha,tips,timeStamp,W,N);
    }

    else
    {
        chosenTips = IOTA(highAlpha,tips,timeStamp,W,N);
    }

    return chosenTips;
}

//TODO : use lambda for find && same for ReconcileTips
void NodeModule::updateTangle(MsgUpdate* Msg, simtime_t attachTime)
{
    pTr_S newTx = new Site;
    newTx->ID = Msg->ID;
    newTx->issuedBy = Msg->issuedBy;
    newTx->issuedTime = attachTime;

    for(auto& tipSelected : Msg->S_approved)
    {
        for(auto& tr : myTangle)
        {
            if(tr->ID.compare(tipSelected) == 0)
            {
                newTx->S_approved.push_back(tr);
                tr->approvedBy.push_back(newTx);

                if(!(tr->isApproved))
                {
                    tr->approvedTime = attachTime;
                    tr->isApproved = true;
                }

                std::map<std::string,pTr_S>::iterator it;

                for(it = myTips.begin(); it != myTips.end();)
                {
                   if(tr->ID.compare(it->first) == 0)
                   {
                       it = myTips.erase(it);
                   }

                   else
                   {
                       it++;
                   }
                }
            }
        }
    }

    myTips.insert({newTx->ID,newTx});
    myTangle.push_back(newTx);
}

void NodeModule::initialize()
{
    std::string file = "./data/Tracking/logNodeModule[" + std::to_string(getId() - 2) + "].txt";
    LOG_SIM.open(file.c_str(),std::ios::app);

    ID = "[" + std::to_string(getId() - 2) + "]";
    powTime = par("powTime");
    prob = par("prob");
    txLimit = getParentModule()->par("transactionLimit");
    NeighborsNumber = getParentModule()->par("NodeNumber");
    NeighborsNumber--;

    genesisBlock = createGenBlock();
    myTangle.push_back(genesisBlock);
    myTips.insert({"Genesis",genesisBlock});

    msgIssue = new cMessage("Trying to issue a new transaction",ISSUE);
    msgUpdate = new cMessage("Broadcasting a new transaction",UPDATE);
    Msg = new MsgUpdate;

    scheduleAt(simTime() + par("trgenRate"), msgIssue);

    LOG_SIM << simTime() << " Initialization complete" << std::endl;
    EV << "Initialization complete" << std::endl;

    LOG_SIM.close();
}

void NodeModule::handleMessage(cMessage * msg)
{
    std::string file = "./data/Tracking/logNodeModule" + ID + ".txt";
    LOG_SIM.open(file.c_str(),std::ios::app);

    if(msg->getKind() == ISSUE)
    {
        bool test = ifIssue(prob);

        EV << "Trying to issue a new transaction" << std::endl;
        EV << "ifIssue result : " << test << std::endl;

        LOG_SIM << simTime() << " Trying to issue a new transaction" << std::endl;
        LOG_SIM << simTime() << " ifIssue result : " << test << std::endl;

        if(test && txCount < txLimit)
        {
            txCount++;
            pTr_S newTx;
            int tipsNb = 0;
            std::string trId = ID + std::to_string(txCount);

            EV << "TSA procedure for " << trId<< std::endl;
            LOG_SIM << simTime() << " TSA procedure for " << trId << std::endl;

            if(strcmp(par("TSA"),"IOTA") == 0)
            {
               std::map<std::string, pTr_S> tipsCopy = giveTips();
               VpTr_S chosenTips = IOTA(par("alpha"),tipsCopy,simTime(),par("W"),par("N"));
               tipsNb = static_cast<int>(chosenTips.size());
               newTx = attach(trId,simTime(),chosenTips);
            }

            if(strcmp(par("TSA"),"GIOTA") == 0)
            {
               std::map<std::string, pTr_S> tipsCopy = giveTips();
               VpTr_S chosenTips = GIOTA(par("alpha"),tipsCopy,simTime(),par("W"),par("N"));
               tipsNb = static_cast<int>(chosenTips.size());
               newTx = attach(trId,simTime(),chosenTips);
            }

            if(strcmp(par("TSA"),"EIOTA") == 0)
            {
                std::map<std::string, pTr_S> tipsCopy = giveTips();
                VpTr_S chosenTips = EIOTA(par("p1"),par("p2"),tipsCopy,simTime(),par("W"),par("N"));
                tipsNb = static_cast<int>(chosenTips.size());
                newTx = attach(trId,simTime(),chosenTips);
            }

            Msg->ID = newTx->ID;
            Msg->issuedBy = newTx->issuedBy;
            Msg->S_approved.clear();

            for(auto approvedTips : newTx->S_approved)
            {
                Msg->S_approved.push_back(approvedTips->ID);
            }

            msgUpdate->setContextPointer(Msg);

            EV << "Pow time = " << tipsNb*powTime<< std::endl;
            EV << "Sending the new transaction to all nodes"<< std::endl;

            LOG_SIM << simTime() << " Pow time = " << tipsNb*powTime << std::endl;
            LOG_SIM << simTime() << " Sending the new transaction to all nodes" << std::endl;

            for(int i = 0; i < NeighborsNumber; i++)
            {
                sendDelayed(msgUpdate->dup(),tipsNb*powTime,"NodeOut",i);
            }

            scheduleAt(simTime() + par("trgenRate"), msgIssue);
        }

        else if(!test && txCount < txLimit)
        {
            LOG_SIM << simTime() << " Prob not higher than test, retrying to issue again" << std::endl;
            EV << "Prob not higher than test, retrying to issue again" << std::endl;

            scheduleAt(simTime() + par("trgenRate"), msgIssue);
        }

        else if(txCount >= txLimit)
        {
            EV << "Number of transactions reached : stopping issuing"<< std::endl;
            LOG_SIM << simTime() << " Number of transactions reached : stopping issuing" << std::endl;
        }

        LOG_SIM.close();
    }

    else if(msg->getKind() == UPDATE)
    {
        EV << "Updating Tangle"<< std::endl;
        LOG_SIM << simTime() << " Updating Tangle" << std::endl;
        MsgUpdate* Msg = (MsgUpdate*) msg->getContextPointer();
        updateTangle(Msg,simTime());
        delete msg;
        LOG_SIM.close();
    }
}

void NodeModule::finish()
{
    std::string file = "./data/Tracking/logNodeModule" + ID + ".txt";
    LOG_SIM.open(file.c_str(),std::ios::app);

    EV << "By NodeModule" + ID << " : Simulation ended - Deleting my local Tangle"<< std::endl;
    LOG_SIM << simTime() << " Simulation ended - Deleting my local Tangle" << std::endl;

    printTangle();
    printTipsLeft();
    DeleteTangle();

    delete msgIssue;
    delete msgUpdate;
    delete Msg;

    LOG_SIM.close();
}
