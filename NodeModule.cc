#include "NodeModule.h"

enum MessageType{SETTING,ISSUE,POW,UPDATE,ParasiteChainAttack,SplittingAttack};

const int ThresholdIdx = 10;

Define_Module(NodeModule);

std::vector<int> NodeModule::readCSV(bool IfExp)
{
    auto env =  cSimulation::getActiveEnvir();
    auto currentRun = env->getConfigEx()->getActiveRunNumber();
    std::vector<int> neib;

    if(IfExp)
    {
        std::fstream file;
        std::string path = "./topologies/exp_CSV/expander" + std::to_string(currentRun) + ".csv";
        file.open(path);

        if(!file.is_open()) throw std::runtime_error("Could not open expander CSV file");

        std::string line;
        int count = 0;

        while(getline(file, line,'\n'))
        {
            if(count == getId() - 2)
            {
                std::istringstream templine(line);
                std::string data;

                bool test = false;

                while(std::getline(templine, data,','))
                {
                  if(test)
                  {
                    neib.push_back(atoi(data.c_str()));
                  }

                  test = true;
                }
            }

            count++;
        }

        file.close();
    }

    else
    {
        std::fstream file;
        std::string path = "./topologies/ws_CSV/watts_strogatz" + std::to_string(currentRun) + ".csv";
        file.open(path,std::ios::in);

        if(!file.is_open()) throw std::runtime_error("Could not open watts strogatz CSV file");

        std::string line;
        int count = 0;

        while(getline(file, line,'\n'))
        {
            if(count == getId() - 2)
            {
                std::istringstream templine(line);
                std::string data;

                bool test = false;

                while(std::getline(templine, data,','))
                {
                  if(test)
                  {
                    neib.push_back(atoi(data.c_str()));
                  }

                  test = true;
                }
            }

            count++;
        }

        file.close();
    }

    return neib;
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
    std::string path = "./data/tracking/TrackerTangle" + ID + ".txt";
    remove(path.c_str());
    file.open(path,std::ios::app);

    for(long unsigned int i = 0; i < myTangle.size(); i++)
    {
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
    std::string path = "./data/tracking/NumberTips" + ID + ".txt";
    //remove(path.c_str());
    file.open(path,std::ios::app);
    file << myTips.size() << std::endl;
    file.close();
}

void NodeModule::stats()
{
    std::fstream file;
    std::string path = "./data/tracking/Stats" + ID + ".txt";
    //remove(path.c_str());
    file.open(path,std::ios::app);
    file << getParentModule()->getName() << ";" << myTangle.size() << ";" << myTips.size() << ";" << myBuffer.size() << std::endl;
    file.close();
}

void NodeModule::debug()
{
    std::fstream file;
    std::string path = "./data/debug/debug" + ID + ".txt";
    remove(path.c_str());
    file.open(path,std::ios::app);
    file << simTime() << " HERE" << std::endl;
    file.close();
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
    int iterAdvances = intuniform(0,tips.size() - 1);
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
        approvesIndex = intuniform(0,current->S_approved.size() - 1);
        current = current->S_approved.at(approvesIndex);
        --count;
    }

    return current;
}

void NodeModule::ReconcileTips(const VpTr_S& removeTips, std::map<std::string,pTr_S>& myTips)
{
    std::map<std::string,pTr_S>::iterator it;

    for(auto& tipSelected : removeTips)
    {
       for(it = myTips.begin(); it != myTips.end();)
       {
           if(tipSelected->ID == it->first)
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

    for(auto& tipSelected : chosen)
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

pTr_S NodeModule::WeightedRandomWalk(pTr_S start, double alphaVal, std::map<std::string, pTr_S>& tips, simtime_t timeStamp, int &walk_time)
{
    int walkCounts = 0;
    pTr_S current = start;

    while(current->isApproved)
    {
        walkCounts++;
        VpTr_S currentView = current->approvedBy;

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
            std::vector<int> sitesWeight;
            int start_weight = ComputeWeight(current,timeStamp);
            std::vector<std::pair<int,double>> sitesProb;

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
            double probWalkChoice = uniform(0.0,1.0);

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
    current->countSelected++;

    return current;
}

pTr_S NodeModule::RandomWalk(pTr_S start, std::map<std::string, pTr_S>& tips, simtime_t timeStamp, int &walk_time)
{
    int walkCounts = 0;
    pTr_S current = start;

    while(current->isApproved)
    {
        walkCounts++;
        VpTr_S currentView = current->approvedBy;

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
            std::vector<std::pair<int,double>> sitesProb;
            double prob = double(1/currentView.size());

            for(int j = 0; j < static_cast<int>(currentView.size()); j++)
            {
               sitesProb.push_back(std::make_pair(j,prob));
            }

            int nextCurrentIndex = 0;
            double probWalkChoice = uniform(0.0,1.0);

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
    current->countSelected++;

    return current;
}

void NodeModule::updateConfidence(double confidence, pTr_S& current)
{
    current->confidence += confidence;

    for(auto& tx : current->approvedBy)
    {
        updateConfidence(confidence, tx);
    }
}

double NodeModule::getavgConfidence(pTr_S current)
{
    double avg = 0.0;

    for(auto tx : current->approvedBy)
    {
        avg += tx->confidence;
        avg += getavgConfidence(tx);
    }

    return avg;
}

pTr_S NodeModule::findConflict(pTr_S tip)
{
    if(tip->ID[0] == '-')
    {
        return tip;
    }

    for(auto adjTx : tip->approvedBy)
    {
        if(!adjTx->isGenesisBlock)
        {
            auto result = findConflict(adjTx);

            if(result != nullptr)
            {
                return result;
            }
        }
    }

    return nullptr;
}

pTr_S NodeModule::IfConflict(pTr_S tip, std::string id)
{
    if(tip->ID == id)
    {
        return tip;
    }

    for(auto adjTx : tip->approvedBy)
    {
        if(!adjTx->isGenesisBlock)
        {
            auto result = IfConflict(adjTx,id);

            if(result != nullptr)
            {
                return result;
            }
        }
    }

    return nullptr;
}

void NodeModule::getConfidence(pTr_S tx, pTr_S& RefTx)
{
    if(!tx->isApproved)
   {
       RefTx->confidence++;
       return;
   }

    for(auto adjTx : tx->S_approved)
    {
       getConfidence(adjTx,RefTx);
    }
}

VpTr_S NodeModule::IOTA(double alphaVal, std::map<std::string, pTr_S>& tips, simtime_t timeStamp, int W, int N)
{
    if(tips.size() == 1)
    {
        VpTr_S tipstoApprove;
        tipstoApprove.push_back(tips.begin()->second);
        tipstoApprove.push_back(tips.begin()->second);
        return tipstoApprove;
    }

    VpTr_S start_sites;
    pTr_S temp_site;
    int backTD;

    for(int i = 0; i < N; i++)
    {
        backTD = intuniform(W,2*W);
        temp_site = getWalkStart(tips,backTD);
        start_sites.push_back(temp_site);
    }

    VpTr_S selected_tips;
    std::vector<std::pair<int,int>> walk_total;
    int walk_time;

    if(alphaVal == 0.0)
    {
        for(int i = 0; i < N; i++)
        {
            auto tip = RandomWalk(start_sites[i],tips,timeStamp,walk_time);
            selected_tips.push_back(tip);
            walk_total.push_back(std::make_pair(i,walk_time));
        }
    }

    else
    {
        for(int i = 0; i < N; i++)
        {
            auto tip = WeightedRandomWalk(start_sites[i],alphaVal,tips,timeStamp,walk_time);
            selected_tips.push_back(tip);
            walk_total.push_back(std::make_pair(i,walk_time));
        }
    }

    std::sort(walk_total.begin(), walk_total.end(),[](const std::pair<int,int> &a, const std::pair<int,int> &b){return a.second < b.second;});

    VpTr_S tipstoApprove;
    tipstoApprove.push_back(selected_tips[walk_total[0].first]);

    for(auto idx : walk_total)
    {
        auto tip = selected_tips[idx.first];

        if(tip->ID != tipstoApprove[0]->ID)
        {
            tipstoApprove.push_back(tip);
            break;
        }
    }

    if(tipstoApprove.size() == 1)
    {
        tipstoApprove.push_back(tipstoApprove[0]);
        return tipstoApprove;
    }

    pTr_S buffer = findConflict(tipstoApprove[0]);

    if(buffer == nullptr)
    {
        buffer = findConflict(tipstoApprove[1]);

        if(buffer == nullptr)
        {
            return tipstoApprove;
        }

        else
        {
            std::string id = buffer->ID;
            id.pop_back();
            pTr_S doubleSpendTx = IfConflict(tipstoApprove[0],id);

            if(doubleSpendTx == nullptr)
            {
                return tipstoApprove;
            }

            else
            {
                tipstoApprove[0]->confidence = 0.0;
                tipstoApprove[1]->confidence = 0.0;

                getConfidence(tipstoApprove[0],tipstoApprove[0]);
                getConfidence(tipstoApprove[1],tipstoApprove[1]);

                std::string DoNotCheck;

                if(tipstoApprove[1]->confidence <= tipstoApprove[0]->confidence)
                {
                    DoNotCheck = tipstoApprove[1]->ID;
                    tipstoApprove.erase(tipstoApprove.begin() + 1);
                }

                else
                {
                    DoNotCheck = tipstoApprove[0]->ID;
                    tipstoApprove.erase(tipstoApprove.begin());
                }

                for(auto idx : walk_total)
                {
                    auto tip = selected_tips[idx.first];

                    if(tip->ID != DoNotCheck && tipstoApprove[0]->ID != tip->ID)
                    {
                        buffer = findConflict(tip);

                        if(buffer == nullptr)
                        {
                            tipstoApprove.push_back(tip);
                            return tipstoApprove;
                        }

                        else
                        {
                            id = buffer->ID;
                            id.pop_back();
                            doubleSpendTx = IfConflict(tipstoApprove[0],id);

                            if(doubleSpendTx == nullptr)
                            {
                                tipstoApprove.push_back(tip);
                                return tipstoApprove;
                            }
                        }
                    }
                }
            }
        }
    }

    else
    {
        std::string id = buffer->ID;
        id.pop_back();

        buffer = findConflict(tipstoApprove[1]);

        if(buffer != nullptr)
        {
            return tipstoApprove;
        }

        else
        {
            pTr_S doubleSpendTx = IfConflict(tipstoApprove[1],id);

            if(doubleSpendTx == nullptr)
            {
                return tipstoApprove;
            }

            else
            {
                tipstoApprove[0]->confidence = 0.0;
                tipstoApprove[1]->confidence = 0.0;

                getConfidence(tipstoApprove[0],tipstoApprove[0]);
                getConfidence(tipstoApprove[1],tipstoApprove[1]);

                std::string DoNotCheck;

                if(tipstoApprove[1]->confidence <= tipstoApprove[0]->confidence)
                {
                    DoNotCheck = tipstoApprove[1]->ID;
                    tipstoApprove.erase(tipstoApprove.begin() + 1);
                }

                else
                {
                    DoNotCheck = tipstoApprove[0]->ID;
                    tipstoApprove.erase(tipstoApprove.begin());
                }

                for(auto idx : walk_total)
                {
                    auto tip = selected_tips[idx.first];

                    if(tip->ID != DoNotCheck && tipstoApprove[0]->ID != tip->ID)
                    {
                        buffer = findConflict(tip);

                        if(buffer == nullptr)
                        {
                            tipstoApprove.push_back(tip);
                            return tipstoApprove;
                        }

                        else
                        {
                            id = buffer->ID;
                            id.pop_back();
                            doubleSpendTx = IfConflict(tipstoApprove[0],id);

                            if(doubleSpendTx == nullptr)
                            {
                                tipstoApprove.push_back(tip);
                                return tipstoApprove;
                            }
                        }
                    }
                }
            }
        }
    }

    if(tipstoApprove.size() == 1)
    {
        tipstoApprove.push_back(tipstoApprove[0]);
    }

   return tipstoApprove;
}

VpTr_S NodeModule::GIOTA(double alphaVal, std::map<std::string, pTr_S>& tips, simtime_t timeStamp, int W, int N)
{
   for(auto& tx : myTangle)
   {
       tx->confidence = 0.0;
       tx->countSelected = 0;
   }

   auto copyTips = tips;
   VpTr_S chosenTips = IOTA(alphaVal,tips,timeStamp,W,N);
   VpTr_S filterTips;

   for(auto it = copyTips.begin(); it != copyTips.end(); ++it)
   {
       auto tip = it->second;

       for(int j = 0; j < static_cast<int>(chosenTips.size()); j++)
       {
           if(!(chosenTips[j]->ID == tip->ID))
           {
               filterTips.push_back(tip);
               break;
           }
       }
   }

   if(filterTips.empty())
   {
       return chosenTips;
   }

   for(auto tip : filterTips)
   {
       for(auto& tx : tip->approvedBy)
       {
           updateConfidence(double(tip->countSelected/N),tx);
       }
   }

   std::vector<std::pair<double,pTr_S>> avgConfTips;

   for(auto tip : filterTips)
   {
       auto avg = getavgConfidence(tip);
       avgConfTips.push_back(std::make_pair(avg,tip));
   }

   std::sort(avgConfTips.begin(), avgConfTips.end(),[](const std::pair<long double,pTr_S> &a, const std::pair<long double,pTr_S> &b){
   return a.first < b.first;});

   chosenTips.push_back(avgConfTips.front().second);
   return chosenTips;
}

VpTr_S NodeModule::EIOTA(double p1, double p2, double lowAlpha, double highAlpha, std::map<std::string,pTr_S>& tips, simtime_t timeStamp, int W, int N)
{
    auto r = uniform(0.0,1.0);

    if(r < p1)
    {
        return IOTA(0.0,tips,timeStamp,W,N);
    }

    else if(p1 <= r && r < p2)
    {
       return IOTA(lowAlpha,tips,timeStamp,W,N);
    }

    return IOTA(highAlpha,tips,timeStamp,W,N);
}

void NodeModule::updateTangle(MsgUpdate* Msg, simtime_t attachTime)
{
    pTr_S newTx = new Site;
    newTx->ID = Msg->ID;
    newTx->issuedBy = Msg->issuedBy;
    newTx->issuedTime = Msg->issuedTime;

    for(auto tipSelected : Msg->S_approved)
    {
        for(auto& tx : myTangle)
        {
            if(tx->ID == tipSelected)
            {
                newTx->S_approved.push_back(tx);
                tx->approvedBy.push_back(newTx);

                if(!(tx->isApproved))
                {
                    tx->approvedTime = attachTime;
                    tx->isApproved = true;
                }

                std::map<std::string,pTr_S>::iterator it;

                for(it = myTips.begin(); it != myTips.end();)
                {
                   if(tx->ID == it->first)
                   {
                       it = myTips.erase(it);
                       break;
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

bool NodeModule::ifAddTangle(std::vector<std::string> S_approved)
{
    for(auto selectedTip : S_approved)
    {
        auto it = std::find_if(myTangle.begin(), myTangle.end(), [&selectedTip](const pTr_S& tx) {return tx->ID == selectedTip;});

        if(it == myTangle.end())
        {
          return false;
        }
    }

    return true;
}

bool NodeModule::IfPresent(std::string txID)
{
    auto it = std::find_if(myTangle.begin(), myTangle.end(), [&txID](const pTr_S& tx) {return tx->ID == txID;});

    if(it != myTangle.end())
    {
      return true;
    }

    auto it2 = std::find_if(myBuffer.begin(), myBuffer.end(), [&txID](const MsgUpdate* msg) {return msg->ID == txID;});

    if(it2 != myBuffer.end())
    {
      return true;
    }

    return false;
}

void NodeModule::updateBuffer()
{
    bool test;

    while(1)
    {
        test = false;
        auto it = myBuffer.begin();

        while(it != myBuffer.end())
        {
            if(ifAddTangle((*it)->S_approved))
            {
                test = true;
                EV << "Updating the Buffer" << std::endl;
                updateTangle((*it),simTime());
                delete (*it);
                it = myBuffer.erase(it);
                break;
            }

            else
            {
                it++;
            }
        }

        if(!test)
        {
            break;
        }
    }
}

void NodeModule::initialize()
{
    ID = "[" + std::to_string(getId() - 2) + "]";
    mean = par("mean");
    powTime = par("powTime");
    txLimit = getParentModule()->par("transactionLimit");

    for(SubmoduleIterator iter(getParentModule()); !iter.end(); iter++)
    {
        NodeModuleNb++;
    }

    genesisBlock = createGenBlock();
    myTangle.push_back(genesisBlock);
    myTips.insert({"Genesis",genesisBlock});

    msgIssue = new cMessage("Issuing a new transaction",ISSUE);
    msgPoW = new cMessage("PoW time",POW);
    msgUpdate = new cMessage("Broadcasting a new transaction",UPDATE);
    MsgP = new MsgPoW;

    if(strcmp(getParentModule()->getName(),"Expander") == 0 || strcmp(getParentModule()->getName(),"WattsStrogatz") == 0)
    {
        EV << "Setting up connections for Watts Strogatz & Expander topologies :" << std::endl;
        std::vector<int> neibIdx;

        if(strcmp(getParentModule()->getName(),"Expander") == 0)
        {
            neibIdx = readCSV(true);
        }

        else
        {
            neibIdx = readCSV(false);
        }

        if(neibIdx.at(0) != -1)
        {
            for(int idx : neibIdx)
            {
                cModule * nodeNeib = getParentModule()->getSubmodule("Nodes",idx);

                if(nodeNeib == nullptr)
                {
                    throw std::runtime_error("Node not found while setting connections for exp & Ws topo");
                }

                cDelayChannel *channel1 = nullptr;
                cDelayChannel *channel2 = nullptr;

                if(getParentModule()->par("ifRandDelay"))
                {
                    double minDelay = getParentModule()->par("minDelay");
                    double maxDelay = getParentModule()->par("maxDelay");
                    double delay = uniform(minDelay,maxDelay);

                    channel1 = cDelayChannel::create("Channel");
                    channel2 = cDelayChannel::create("Channel");

                    channel1->setDelay(delay);
                    channel2->setDelay(delay);
                }

                else
                {
                    channel1 = cDelayChannel::create("Channel");
                    channel2 = cDelayChannel::create("Channel");

                    channel1->setDelay(getParentModule()->par("delay"));
                    channel2->setDelay(getParentModule()->par("delay"));
                }

                setGateSize("NodeOut",gateSize("NodeOut") + 1);
                setGateSize("NodeIn",gateSize("NodeIn") + 1);

                nodeNeib->setGateSize("NodeIn",nodeNeib->gateSize("NodeIn") + 1);
                nodeNeib->setGateSize("NodeOut",nodeNeib->gateSize("NodeOut") + 1);

                auto gOut = gate("NodeOut",gateSize("NodeOut") - 1);
                auto gIn = nodeNeib->gate("NodeIn", nodeNeib->gateSize("NodeIn") - 1);

                gOut->connectTo(gIn,channel1);

                gOut = nodeNeib->gate("NodeOut", nodeNeib->gateSize("NodeOut") - 1);
                gIn = gate("NodeIn",gateSize("NodeIn") - 1);

                gOut->connectTo(gIn,channel2);

                EV << getId() - 2 << " <---> " << nodeNeib->getId() - 2 << std::endl;
            }
        }

        auto msgSetting = new cMessage("Connections are set !",SETTING);
        EV << "Connections are set !" << std::endl;
        scheduleAt(simTime(), msgSetting);
    }

    else
    {
        NeighborsNumber = gateSize("NodeOut");

        if(getParentModule()->par("ifRandDelay"))
        {
            for(int i = 0; i < NeighborsNumber; i++)
            {
                cGate *g = gate("NodeOut",i);
                cDelayChannel *channel = check_and_cast<cDelayChannel*>(g->getChannel());
                channel->setDelay(9223372.0);
            }

            auto msgSetting = new cMessage("Setting random delays",SETTING);
            EV << "Setting random delays !" << std::endl;
            scheduleAt(simTime(), msgSetting);
        }

        else
        {
            EV << "Initialization complete" << std::endl;
            scheduleAt(simTime() + exponential(mean), msgIssue);
        }
    }
}

void NodeModule::handleMessage(cMessage * msg)
{
    if(msg->getKind() == SETTING)
    {
        delete msg;

        if(strcmp(getParentModule()->getName(),"Expander") == 0 || strcmp(getParentModule()->getName(),"WattsStrogatz") == 0)
        {
            NeighborsNumber = gateSize("NodeOut");
        }

        else
        {
            cModule *mymodule = getParentModule()->getSubmodule("Nodes",getId() - 2);

            if(mymodule == nullptr)
            {
                throw std::runtime_error("Node not found while setting random delays");
            }

            double minDelay = getParentModule()->par("minDelay");
            double maxDelay = getParentModule()->par("maxDelay");

            for(int i = 0; i < NeighborsNumber; i++)
            {
                cGate *g1 = gate("NodeOut",i);
                cModule *Adjmodule;
                double delay;
                double maxdbl = 9223372.0;

                for(SubmoduleIterator iter(getParentModule()); !iter.end(); iter++)
                {
                    Adjmodule = *iter;

                    if(g1->pathContains(Adjmodule) && Adjmodule->getId() != getId())
                    {
                        break;
                    }
                }

                cDelayChannel *channel1 = check_and_cast<cDelayChannel*>(g1->getChannel());
                delay = channel1->getDelay().dbl();

                for(int j = 0; j < Adjmodule->gateSize("NodeOut"); j++)
                {
                    cGate *g2 = Adjmodule->gate("NodeOut",j);

                    if(g2->pathContains(mymodule))
                    {
                        cDelayChannel *channel2 = check_and_cast<cDelayChannel*>(g2->getChannel());

                        if(channel2->getDelay() != maxdbl && delay == maxdbl)
                        {
                            delay = channel2->getDelay().dbl();
                            channel1->setDelay(delay);
                        }

                        else if(channel2->getDelay() == maxdbl && delay != maxdbl)
                        {
                            channel2->setDelay(delay);
                        }

                        else
                        {
                            delay = uniform(minDelay,maxDelay);
                            channel1->setDelay(delay);
                            channel2->setDelay(delay);
                        }

                        break;
                    }
                }
            }
        }

        EV << "Initialization complete" << std::endl;
        scheduleAt(simTime() + exponential(mean), msgIssue);
    }

    else if(msg->getKind() == ISSUE)
    {
        EV << "Issuing a new transaction" << std::endl;

        if(txCount < txLimit)
        {
            txCount++;
            VpTr_S chosenTips;
            int tipsNb = 0;
            std::string trId = "null";

            if(ID == "[0]")
            {
                if(!IfDoubleSpend && ThresholdIdx < myTangle.size())
                {
                    bool ifAttack = false;

                    for(long unsigned int i = 1; i < ThresholdIdx; i++)
                    {
                        auto tx = myTangle[i];

                        if(tx->ID != "Genesis" && tx->isApproved)
                        {
                            tx->confidence = 0.0;
                            getConfidence(tx,tx);

                            double Conflvl = getParentModule()->par("Confidencelvl");

                            if(tx->confidence/myTips.size() >= Conflvl)
                            {
                                trId = "-" + tx->ID;
                                IfDoubleSpend = true;
                                ifAttack = true;
                                EV << "Launching a double spending attack !" << std::endl;
                                break;
                            }
                        }
                    }

                    if(!ifAttack)
                    {
                        trId = ID + std::to_string(txCount);
                    }
                }

                else
                {
                    trId = ID + std::to_string(txCount);
                }
            }

            else
            {
                trId = ID + std::to_string(txCount);
            }

            EV << "TSA procedure for " << trId << std::endl;

            if(strcmp(par("TSA"),"IOTA") == 0)
            {
               auto tipsCopy = myTips;
               chosenTips = IOTA(par("alpha"),tipsCopy,simTime(),par("W"),par("N"));
               tipsNb = static_cast<int>(chosenTips.size());
            }

            if(strcmp(par("TSA"),"GIOTA") == 0)
            {
               auto tipsCopy = myTips;
               chosenTips = GIOTA(par("alpha"),tipsCopy,simTime(),par("W"),par("N"));
               tipsNb = static_cast<int>(chosenTips.size());
            }

            if(strcmp(par("TSA"),"EIOTA") == 0)
            {
                auto tipsCopy = myTips;
                chosenTips = EIOTA(par("p1"),par("p2"),par("lowAlpha"),par("highAlpha"),tipsCopy,simTime(),par("W"),par("N"));
                tipsNb = static_cast<int>(chosenTips.size());
            }

            EV << "Chosen Tips : ";

            for(auto tips : chosenTips)
            {
                EV << tips->ID << " ";
            }

            EV << std::endl;

            MsgP->ID = trId;
            MsgP->chosen = chosenTips;
            msgPoW->setContextPointer(MsgP);

            if(chosenTips[0]->ID == chosenTips[1]->ID)
            {
                tipsNb = 1;
            }

            EV << "Pow time = " << tipsNb*powTime<< std::endl;
            scheduleAt(simTime() + tipsNb*powTime, msgPoW);
        }

        else if(txCount >= txLimit)
        {
            EV << "Number of transactions reached : stopping issuing" << std::endl;
        }
    }

    else if(msg->getKind() == POW)
    {
        MsgPoW* Msg = (MsgPoW*) msg->getContextPointer();
        pTr_S newTx = attach(Msg->ID,simTime(),Msg->chosen);

        EV << "Pow time finished for " << Msg->ID << std::endl;
        EV << " Sending " << newTx->ID << " to all nodes" << std::endl;

        for(int i = 0; i < NeighborsNumber; i++)
        {
            MsgUpdate * MsgU = new MsgUpdate;

            MsgU->ID = newTx->ID;
            MsgU->issuedBy = newTx->issuedBy;
            MsgU->issuedTime = newTx->issuedTime;

            for(auto approvedTips : newTx->S_approved)
            {
                MsgU->S_approved.push_back(approvedTips->ID);
            }

            msgUpdate->setContextPointer(MsgU);

            send(msgUpdate->dup(),"NodeOut",i);
        }

        scheduleAt(simTime() + exponential(mean), msgIssue);
    }

    else if(msg->getKind() == UPDATE)
    {
       MsgUpdate* Msg = (MsgUpdate*) msg->getContextPointer();

       if(IfPresent(Msg->ID))
       {
           EV << Msg->ID << " is already present"<< std::endl;
           delete Msg;
           delete msg;
       }

       else
       {
           auto Sender = msg->getSenderModule();

           EV << "Received a new transaction " << Msg->ID << std::endl;

           if(NodeModuleNb - 1 != NeighborsNumber)
           {
               EV << " Sending " << Msg->ID << " to all nodes" << std::endl;

               for(int i = 0; i < NeighborsNumber; i++)
               {
                   cGate *g = gate("NodeOut",i);

                   if(!g->pathContains(Sender))
                   {
                       MsgUpdate * MsgU = new MsgUpdate;

                       MsgU->ID = Msg->ID;
                       MsgU->issuedBy = Msg->issuedBy;
                       MsgU->issuedTime = Msg->issuedTime;

                       for(auto approvedTipsID : Msg->S_approved)
                       {
                           MsgU->S_approved.push_back(approvedTipsID);
                       }

                       msgUpdate->setContextPointer(MsgU);

                       send(msgUpdate->dup(),"NodeOut",i);
                   }
               }
           }

           updateBuffer();

           if(ifAddTangle(Msg->S_approved))
           {
               EV << "Updating the Tangle" << std::endl;
               updateTangle(Msg,simTime());
               updateBuffer();
               delete Msg;
           }

           else
           {
               EV << "Adding to the Buffer" << std::endl;
               myBuffer.push_back(Msg);
           }

           delete msg;
       }
    }
}

void NodeModule::finish()
{
    EV << "By NodeModule" + ID << " : Simulation ended - Deleting my local Tangle" << std::endl;

    printTangle();
    //printTipsLeft();
    stats();
    DeleteTangle();

    delete msgIssue;
    delete msgPoW;
    delete msgUpdate;
    delete MsgP;
}
