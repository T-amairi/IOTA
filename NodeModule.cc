#include "NodeModule.h"

enum MessageType{SETTING,ISSUE,POW,UPDATE,ParasiteChainAttack,SplittingAttack,InitializationSplittingAttack,MaintainBalance};

Define_Module(NodeModule);

std::vector<int> NodeModule::readCSV(bool IfExp)
{
    auto env =  cSimulation::getActiveEnvir();
    auto currentRun = env->getConfigEx()->getActiveRunNumber();
    std::vector<int> neib;
    std::fstream file;
    std::string path;

    if(IfExp)
    {
        path = "./topologies/exp_CSV/expander" + std::to_string(currentRun) + ".csv";
    }

    else
    {
        path = "./topologies/ws_CSV/watts_strogatz" + std::to_string(currentRun) + ".csv";
    }

    file.open(path,std::ios::in);

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

void NodeModule::printChain()
{
    std::fstream file;
    std::string path = "./data/tracking/Chain" + ID + ".txt";
    remove(path.c_str());
    file.open(path,std::ios::app);

    file << getId() - 2 << std::endl;

    for(auto DoubleSpendTx : myDoubleSpendTx)
    {
       for(auto tx : myTangle)
       {
           auto it = tx->pathID.find(DoubleSpendTx->ID);

           if(it != tx->pathID.end())
           {
               file << tx->ID << std::endl;
           }
       }
    }

    file.close();

    path = "./data/tracking/LegitChain" + ID + ".txt";
    remove(path.c_str());
    file.open(path,std::ios::app);

    file << getId() - 2 << std::endl;

    for(auto DoubleSpendTx : myDoubleSpendTx)
    {
       for(auto tx : myTangle)
       {
           auto id = DoubleSpendTx->ID;
           id.erase(id.begin());
           auto it = tx->pathID.find(id);

           if(it != tx->pathID.end())
           {
               file << tx->ID << std::endl;
           }
       }
    }

    file.close();
}

void NodeModule::PercentDiffBranch()
{
    std::fstream file;
    std::string path = "./data/tracking/DiffBranch" + ID + ".txt";
    //remove(path.c_str());
    file.open(path,std::ios::app);

    if(branch2.size() == 0)
    {
        file << "null" << std::endl;
    }

    else
    {
        file << std::abs((double)(branch1.size() - branch2.size())*100/branch2.size()) << std::endl;
    }

    file.close();
}

void NodeModule::DiffTxChain()
{
    std::fstream file;
    std::string path = "./data/tracking/DiffTxChain" + ID + ".txt";
    //remove(path.c_str());
    file.open(path,std::ios::app);

    if(myDoubleSpendTx.size() == 0)
    {
        file << "null" << std::endl;
    }

    else
    {
        double prop = par("PropComputingPower");
        file << prop << ";";

        for(auto DoubleSpendTx : myDoubleSpendTx)
        {
            file << DoubleSpendTx->ID << ";";

            int c1 = 0;
            int c2 = 0;

            for(auto tx : myTangle)
            {
                if(tx->issuedTime >= DoubleSpendTx->issuedTime && tx->ID != DoubleSpendTx->ID)
                {
                    auto it = tx->pathID.find(DoubleSpendTx->ID);

                    if(it != tx->pathID.end())
                    {
                        c1++;
                    }

                    else
                    {
                        c2++;
                    }
                }
            }

           file << c1 - c2 << std::endl;
        }
    }

    file.close();
}

int NodeModule::_computeWeight(VpTr_S& visited, pTr_S& current)
{
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
            weight += 1 + _computeWeight(visited, current->approvedBy.at(i));
        }
    }

    return weight;
}

int NodeModule::ComputeWeight(pTr_S tr)
{
    VpTr_S visited;
    int weight = _computeWeight(visited, tr);

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

pTr_S NodeModule::getWalkStart(int backTrackDist)
{
    int iterAdvances = intuniform(0,myTips.size() - 1);
    auto beginIter = myTips.begin();

    if(myTips.size() > 1)
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
    new_tips->pathID = getpathID(chosen);
    new_tips->pathID.insert(new_tips->ID);
    ReconcileTips(chosen,myTips);

    myTangle.push_back(new_tips);
    myTips.insert({new_tips->ID,new_tips});

    if(ID[0] == '-')
    {
        myDoubleSpendTx.push_back(new_tips);
    }

    return new_tips;
}

pTr_S NodeModule::WeightedRandomWalk(pTr_S start, double alphaVal, int &walk_time)
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
            int start_weight = ComputeWeight(current);
            std::vector<double> p;

            double sum_exp = 0.0;
            int weight;

            for(int j = 0; j < static_cast<int>(currentView.size()); j++)
            {
                weight = ComputeWeight(currentView[j]);
                sum_exp = sum_exp + double(exp(double(-alphaVal*(start_weight - weight))));
                sitesWeight.push_back(weight);
            }

            double prob;

            for(int j = 0; j < static_cast<int>(sitesWeight.size()); j++)
            {
               prob = double(exp(double(-alphaVal*(start_weight - sitesWeight[j]))));
               prob = prob/sum_exp;
               p.push_back(prob);
            }

            int nextCurrentIndex = 0;
            double probWalkChoice = uniform(0.0,1.0);

            auto p1 = p;
            std::rotate(p1.rbegin(), p1.rbegin() + 1, p1.rend());
            p1[0] = 0.0;

            std::vector<double> cs;
            cs.resize(p.size());
            std::vector<double> cs1;
            cs1.resize(p1.size());

            std::partial_sum(p.begin(),p.end(),cs.begin());
            std::partial_sum(p1.begin(),p1.end(),cs1.begin());

            for(int k = 0; k < static_cast<int>(cs.size()); k++)
            {
                if(probWalkChoice > cs1[k] && probWalkChoice < cs[k])
                {
                    nextCurrentIndex = k;
                    break;
                }
            }

            current = currentView[nextCurrentIndex];
        }
    }

    walk_time = walkCounts;
    current->countSelected++;

    return current;
}

pTr_S NodeModule::RandomWalk(pTr_S start, int &walk_time)
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
            std::vector<double> p;
            double prob = double(1/currentView.size());

            for(int j = 0; j < static_cast<int>(currentView.size()); j++)
            {
               p.push_back(prob);
            }

            int nextCurrentIndex = 0;
            double probWalkChoice = uniform(0.0,1.0);

            auto p1 = p;
            std::rotate(p1.rbegin(), p1.rbegin() + 1, p1.rend());
            p1[0] = 0.0;

            std::vector<double> cs;
            cs.resize(p.size());
            std::vector<double> cs1;
            cs1.resize(p1.size());

            std::partial_sum(p.begin(),p.end(),cs.begin());
            std::partial_sum(p1.begin(),p1.end(),cs1.begin());

            for(int k = 0; k < static_cast<int>(cs.size()); k++)
            {
                if(probWalkChoice > cs1[k] && probWalkChoice < cs[k])
                {
                    nextCurrentIndex = k;
                    break;
                }
            }

            current = currentView[nextCurrentIndex];
        }
    }

    walk_time = walkCounts;
    current->countSelected++;

    return current;
}

std::unordered_set<std::string> NodeModule::getpathID(VpTr_S chosenTips)
{
    std::unordered_set<std::string> pathID;

    if(chosenTips.size() == 1)
    {
        pathID = chosenTips[0]->pathID;
        return pathID;
    }

    else if(chosenTips[0]->pathID.size() > chosenTips[1]->pathID.size())
    {
        pathID = chosenTips[0]->pathID;
        pathID.insert(chosenTips[1]->pathID.begin(),chosenTips[1]->pathID.end());
    }

    else
    {
        pathID = chosenTips[1]->pathID;
        pathID.insert(chosenTips[0]->pathID.begin(),chosenTips[0]->pathID.end());
    }

    return pathID;
}

std::tuple<bool,std::string> NodeModule::IfLegitTip(std::unordered_set<std::string> path)
{
    std::string id;

    for(auto it = path.begin(); it != path.end(); it++)
    {
        auto txID = (*it);

        if(txID[0] == '-')
        {
            id = (*it);
            break;
        }
    }

    if(id.empty())
    {
        auto res = std::make_tuple(true,id);
        return res;
    }

    id.erase(id.begin());

    for(auto it = path.begin(); it != path.end(); it++)
    {
        if((*it) == id)
        {
            auto res = std::make_tuple(false,id);
            return res;
        }
    }

    auto res = std::make_tuple(true,id);
    return res;
}

bool NodeModule::IfConflict(std::tuple<bool,std::string> tup1, std::tuple<bool,std::string> tup2, std::unordered_set<std::string> path1, std::unordered_set<std::string> path2)
{
    if(!std::get<0>(tup1) || !std::get<0>(tup2))
    {
        return true;
    }

    else if(std::get<1>(tup1).empty() && std::get<1>(tup2).empty())
    {
        return false;
    }

    else if(!std::get<1>(tup1).empty() && !std::get<1>(tup2).empty())
    {
        return false;
    }

    else if(std::get<1>(tup1).empty() && !std::get<1>(tup2).empty())
    {
        auto it = path1.find(std::get<1>(tup2));

        if(it == path1.end())
        {
            return false;
        }
    }

    else if(!std::get<1>(tup1).empty() && std::get<1>(tup2).empty())
    {
        auto it = path2.find(std::get<1>(tup1));

        if(it == path2.end())
        {
            return false;
        }
    }

    return true;
}

int NodeModule::getConfidence(std::string id)
{
    int count = 0;

    for(auto key : myTips)
    {
        auto tip = key.second;
        auto it = tip->pathID.find(id);

        if(it != tip->pathID.end())
        {
            count++;
        }
    }

    return count;
}

VpTr_S NodeModule::IOTA(double alphaVal, int W, int N)
{
    if(myTips.size() == 1)
    {
        VpTr_S tipstoApprove;
        tipstoApprove.push_back(myTips.begin()->second);
        return tipstoApprove;
    }

    std::vector<std::tuple<pTr_S,int,int>> selected_tips;
    int walk_time;

    for(int i = 0; i < N; i++)
    {
        pTr_S tip;
        int w = intuniform(W,2*W);
        pTr_S startTx = getWalkStart(w);

        if(alphaVal == 0.0)
        {
            tip = RandomWalk(startTx,walk_time);
        }

        else
        {
            tip = WeightedRandomWalk(startTx,alphaVal,walk_time);
        }

       bool ifFind = false;

        for(auto& Tip : selected_tips)
        {
            if(std::get<0>(Tip)->ID == tip->ID)
            {
                std::get<1>(Tip)++;
                ifFind = true;
                break;
            }
        }

        if(!ifFind)
        {
            selected_tips.push_back(std::make_tuple(tip,1,walk_time));
        }
    }

    std::sort(selected_tips.begin(), selected_tips.end(), [](const std::tuple<pTr_S,int,int>& tip1, const std::tuple<pTr_S,int,int>& tip2){return std::get<1>(tip1) > std::get<1>(tip2);});

    EV << "Selected tips :\n";

    for(auto tup : selected_tips)
    {
        EV << std::get<0>(tup)->ID << " chosen time : " << std::get<1>(tup) << " walk time : " << std::get<2>(tup) << std::endl;
    }

    VpTr_S tipstoApprove;

    if(selected_tips.size() == 1)
    {
        auto tup = IfLegitTip(std::get<0>(selected_tips[0])->pathID);

        if(std::get<0>(tup))
        {
            tipstoApprove.push_back(std::get<0>(selected_tips[0]));
        }

        return tipstoApprove;
    }

    std::vector<std::tuple<pTr_S,int,int>> legitTips;
    std::set<std::string> DoNotCheck;
    std::tuple<bool,std::string> tup1;
    std::tuple<bool,std::string> tup2;

    for(auto tipTup : selected_tips)
    {
        auto tip = std::get<0>(tipTup);

        if(legitTips.empty())
        {
            tup1 = IfLegitTip(tip->pathID);

            if(std::get<0>(tup1))
            {
                legitTips.push_back(tipTup);
            }

            else
            {
                DoNotCheck.insert(tip->ID);
            }
        }

        else
        {
            tup2 = IfLegitTip(tip->pathID);

            if(std::get<0>(tup2))
            {
                legitTips.push_back(tipTup);
                break;
            }

            else
            {
                DoNotCheck.insert(tip->ID);
            }
        }
    }

    if(legitTips.size() == 1)
    {
        tipstoApprove.push_back(std::get<0>(legitTips[0]));
        return tipstoApprove;
    }

    if(!IfConflict(tup1,tup2,std::get<0>(legitTips[0])->pathID,std::get<0>(legitTips[1])->pathID))
    {
        tipstoApprove.push_back(std::get<0>(legitTips[0]));
        tipstoApprove.push_back(std::get<0>(legitTips[1]));
        return tipstoApprove;
    }

    int conf1 = getConfidence(std::get<0>(legitTips[0])->ID);
    int conf2 = getConfidence(std::get<0>(legitTips[1])->ID);

    if(conf1 >= conf2)
    {
        tipstoApprove.push_back(std::get<0>(legitTips[0]));
        DoNotCheck.insert(std::get<0>(legitTips[1])->ID);
    }

    else
    {
        tipstoApprove.push_back(std::get<0>(legitTips[1]));
        DoNotCheck.insert(std::get<0>(legitTips[0])->ID);
        tup1 = tup2;
    }

    legitTips.clear();

    for(auto tipTup : selected_tips)
    {
        auto tip = std::get<0>(tipTup);

        if(tip->ID != tipstoApprove[0]->ID)
        {
            auto it = DoNotCheck.find(tip->ID);

            if(it == DoNotCheck.end())
            {
                tup2 = IfLegitTip(tip->pathID);

                if(!IfConflict(tup1,tup2,tipstoApprove[0]->pathID,tip->pathID))
                {
                    tipstoApprove.push_back(tip);
                    break;
                }
            }
        }
    }

    return tipstoApprove;
}

VpTr_S NodeModule::GIOTA(double alphaVal, int W, int N)
{
    for(auto& tx : myTangle)
    {
        tx->confidence = 0.0;
        tx->countSelected = 0;
    }

    VpTr_S chosenTips = IOTA(alphaVal,W,N);

    if(chosenTips.size() == 1)
    {
        return chosenTips;
    }

    VpTr_S filterTips;

    for(auto it = myTips.begin(); it != myTips.end(); ++it)
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
        if(tip->countSelected != 0)
        {
           for(auto id : tip->pathID)
            {
                if(id != tip->ID)
                {
                    auto it = std::find_if(myTangle.begin(), myTangle.end(), [&id](const pTr_S& tx){return tx->ID == id;});
                    (*it)->confidence += double(tip->countSelected/N);
                }
            }
        }
    }

    std::vector<std::pair<double,pTr_S>> avgConfTips;

    for(auto tip : filterTips)
    {
        double avg = 0.0;

        for(auto id : tip->pathID)
        {
            if(id != tip->ID)
            {
                auto it = std::find_if(myTangle.begin(), myTangle.end(), [&id](const pTr_S& tx){return tx->ID == id;});
                avg += (*it)->confidence;
            }
        }

        avgConfTips.push_back(std::make_pair(avg,tip));
    }

    std::sort(avgConfTips.begin(), avgConfTips.end(),[](const std::pair<long double,pTr_S> &a, const std::pair<long double,pTr_S> &b){return a.first < b.first;});

    auto tup1 = IfLegitTip(chosenTips[0]->pathID);
    auto tup2 = IfLegitTip(chosenTips[1]->pathID);

    for(auto pair : avgConfTips)
    {
        auto tip = pair.second;

        if(tip->ID != chosenTips[0]->ID && tip->ID != chosenTips[1]->ID)
        {
            auto tup3 = IfLegitTip(tip->pathID);

            if(std::get<0>(tup3))
            {
                if(!IfConflict(tup1,tup3,chosenTips[0]->pathID,tip->pathID) && !IfConflict(tup2,tup3,chosenTips[1]->pathID,tip->pathID))
                {
                    chosenTips.push_back(tip);
                    break;
                }
            }
        }
    }

    return chosenTips;
}

VpTr_S NodeModule::EIOTA(double p1, double p2, double lowAlpha, double highAlpha, int W, int N)
{
    auto r = uniform(0.0,1.0);

    if(r < p1)
    {
        return IOTA(0.0,W,N);
    }

    else if(p1 <= r && r < p2)
    {
       return IOTA(lowAlpha,W,N);
    }

    return IOTA(highAlpha,W,N);
}

void NodeModule::updateTangle(MsgUpdate* Msg)
{
    pTr_S newTx = new Site;
    newTx->ID = Msg->ID;
    newTx->issuedBy = Msg->issuedBy;
    newTx->issuedTime = simTime();

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
                    tx->approvedTime = simTime();
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

    newTx->pathID = getpathID(newTx->S_approved);
    newTx->pathID.insert(newTx->ID);

    if(newTx->ID[0] == '-')
    {
        myDoubleSpendTx.push_back(newTx);
    }

    if(par("SplittingAttack") && strcmp(par("AttackID"),ID.c_str()) == 0 && !IfIniSP)
    {
        updateBranches(newTx);
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
                EV << "Updating the Buffer : " << (*it)->ID << " can be added" << std::endl;
                updateTangle((*it));
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

bool NodeModule::IfAttackStage()
{
    int count = NodeModuleNb - 1;
    double AttackStage = par("AttackStage");

    for(SubmoduleIterator iter(getParentModule()); !iter.end(); iter++)
    {
        cModule *submodule = *iter;

        if(submodule->getId() != getId())
        {
            NodeModule* node = check_and_cast<NodeModule*>(submodule);

            if(node->txCount >= node->txLimit*AttackStage)
            {
                count--;
            }
        }
    }

    if(count == 0)
    {
        return true;
    }

    return false;
}

VpTr_S NodeModule::getParasiteChain(pTr_S RootTip, std::string TargetID, int ChainLength, int NbTipsChain)
{
    auto RootChain = createSite("-" + TargetID);
    RootChain->pathID = RootTip->pathID;
    RootChain->pathID.insert(RootChain->ID);
    RootChain->S_approved.push_back(RootTip);

    RootTip->approvedBy.push_back(RootChain);
    RootTip->isApproved = true;
    RootTip->approvedTime = simTime();

    VpTr_S temp;
    temp.push_back(RootTip);
    ReconcileTips(temp,myTips);
    temp.clear();

    VpTr_S TheChain;
    TheChain.push_back(RootChain);
    myDoubleSpendTx.push_back(RootChain);

    for(int i = 0; i < ChainLength; i++)
    {
        if(i == 0)
        {
            auto NodeChain = createSite(ID + std::to_string(txCount));
            NodeChain->S_approved.push_back(RootChain);
            NodeChain->pathID = RootChain->pathID;
            NodeChain->pathID.insert(NodeChain->ID);

            RootChain->approvedBy.push_back(NodeChain);
            RootChain->approvedTime = simTime();
            RootChain->isApproved = true;

            TheChain.push_back(NodeChain);
        }

        else
        {
            txCount++;

            auto NodeChain = createSite(ID + std::to_string(txCount));
            NodeChain->S_approved.push_back(TheChain.back());
            NodeChain->pathID = TheChain.back()->pathID;
            NodeChain->pathID.insert(NodeChain->ID);

            TheChain.back()->approvedBy.push_back(NodeChain);
            TheChain.back()->approvedTime = simTime();
            TheChain.back()->isApproved = true;

            TheChain.push_back(NodeChain);
        }
    }

    auto BackChain = TheChain.back();

    for(int i = 0; i < NbTipsChain; i++)
    {
        txCount++;

        auto TipChain = createSite(ID + std::to_string(txCount));

        if(i == 0)
        {
            TipChain->S_approved.push_back(BackChain);
            TipChain->pathID = BackChain->pathID;
            TipChain->pathID.insert(TipChain->ID);

            BackChain->approvedBy.push_back(TipChain);
            BackChain->approvedTime = simTime();
            BackChain->isApproved = true;

            TheChain.push_back(TipChain);
        }

        else
        {
            TipChain->S_approved.push_back(BackChain);
            TipChain->pathID = BackChain->pathID;
            TipChain->pathID.insert(TipChain->ID);

            BackChain->approvedBy.push_back(TipChain);

            TheChain.push_back(TipChain);
        }
    }

    return TheChain;
}

void NodeModule::iniSplittingAttack()
{
    txCount++;
    auto tx1 = createSite(ID + std::to_string(txCount));
    txCount++;
    auto tx2 = createSite(ID + std::to_string(txCount));

    auto tip1 = branch1[0];
    auto tip2 = branch2[0];

    tx1->S_approved.push_back(tip1);
    tx2->S_approved.push_back(tip2);

    tip1->approvedBy.push_back(tx1);
    tip2->approvedBy.push_back(tx2);

    if(!tip1->isApproved)
    {
        tip1->isApproved = true;
        tip1->approvedTime = simTime();
    }

    if(!tip2->isApproved)
    {
        tip2->isApproved = true;
        tip2->approvedTime = simTime();
    }

    branch1.push_back(tx1);
    branch2.push_back(tx2);
}

void NodeModule::updateBranches(pTr_S newTx)
{
    auto it = newTx->pathID.find(branch1[0]->ID);

    if(it != newTx->pathID.end())
    {
        branch1.push_back(newTx);
    }

    else
    {
        it = newTx->pathID.find(branch2[0]->ID);

        if(it != newTx->pathID.end())
        {
            branch2.push_back(newTx);
        }
    }
}

pTr_S NodeModule::MaintainingBalance(int whichBranch)
{
    VpTr_S Vtips;
    VpTr_S toIterOver;

    if(whichBranch == 2)
    {
        EV << "Chosen branche to be balanced : 2\n";
        toIterOver =  branch2;
    }

    else
    {
        EV << "Chosen branche to be balanced : 1\n";
        toIterOver = branch1;
    }

    int countTips = 0;

    for(auto tx : toIterOver)
    {
        if(!tx->isApproved)
        {
            countTips++;
        }
    }

    bool Iftip = countTips <= NodeModuleNb - 1;

    if(Iftip)
    {
        for(auto tx : toIterOver)
        {
           if(tx->isApproved)
           {
               Vtips.push_back(tx);
           }
        }
    }

    else
    {
        for(auto tx : toIterOver)
        {
           if(!tx->isApproved)
           {
               Vtips.push_back(tx);
           }
        }
    }

    txCount++;
    auto tx = createSite(ID + std::to_string(txCount));

    int r = intuniform(0,Vtips.size() - 1);
    auto tip = Vtips[r];
    Vtips.erase(Vtips.begin() + r);

    tx->pathID = tip->pathID;
    tx->pathID.insert(tx->ID);
    tx->S_approved.push_back(tip);

    tip->approvedBy.push_back(tx);
    tip->isApproved = true;
    tip->approvedTime = simTime();

    if(!Iftip)
    {
        VpTr_S temp;
        temp.push_back(tip);
        ReconcileTips(temp,myTips);
    }

    if(!Vtips.empty())
    {
        r = intuniform(0,Vtips.size() - 1);
        tip = Vtips[r];
        Vtips.erase(Vtips.begin() + r);

        tx->pathID.insert(tip->pathID.begin(),tip->pathID.end());
        tx->S_approved.push_back(tip);

        tip->approvedBy.push_back(tx);
        tip->isApproved = true;
        tip->approvedTime = simTime();

        if(!Iftip)
        {
            VpTr_S temp;
            temp.push_back(tip);
            ReconcileTips(temp,myTips);
        }
    }

    if(whichBranch == 2)
    {
        branch2.push_back(tx);
    }

    else
    {
        branch1.push_back(tx);
    }

    myTangle.push_back(tx);
    myTips.insert({tx->ID,tx});
    return tx;
}

void NodeModule::IfNodesfinished()
{
    int count = NodeModuleNb - 1;

    for(SubmoduleIterator iter(getParentModule()); !iter.end(); iter++)
    {
        cModule *submodule = *iter;

        if(submodule->getId() != getId())
        {
            NodeModule* node = check_and_cast<NodeModule*>(submodule);

            if(node->txCount >= node->txLimit)
            {
                count--;
            }
        }
    }

    if(count == 0)
    {
        IfSimFinished = true;
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
                    throw std::runtime_error("Node not found while setting connections for exp & Ws topo : check the number of nodes set in the python script and in the .ned file, they have to be identical !");
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

            for(int i = 0; i < NeighborsNumber; i++)
            {
               cGate *g = gate("NodeOut",i);
               auto channel = g->getChannel();
               channel->callInitialize();
            }
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

    else if(msg->getKind() == ParasiteChainAttack)
    {
        delete msg;

        EV << "Building the parasite chain" << std::endl;

        VpTr_S cpyTx;
        bool test1 = false;
        bool test2 = false;

        for(auto tx : myTangle)
        {
            std::string id = par("AttackID");

            if(tx->ID.find(id) != std::string::npos && tx->approvedBy.size() > 2)
            {
                cpyTx.push_back(tx);
            }
        }

        std::sort(cpyTx.begin(), cpyTx.end(), [](const pTr_S& tx1, const pTr_S& tx2){return tx1->issuedTime > tx2->issuedTime;});
        pTr_S TargetTx;

        if(cpyTx.empty())
        {
            EV << "Can not find a legit transaction for the chain, retrying later" << std::endl;
            IfAttack = false;
            scheduleAt(simTime() + exponential(mean), msgIssue);
        }

        else
        {
            test1 = true;
            TargetTx = cpyTx[0];
            cpyTx.clear();
        }

        pTr_S RootTx;
        std::vector<std::pair<int,pTr_S>> cpyTxLegit;

        if(test1)
        {
            for(auto tx : myTangle)
            {
                if(!tx->isGenesisBlock && tx->approvedBy.size() > 2)
                {
                    cpyTx.push_back(tx);
                }
            }

            for(auto tx : cpyTx)
            {
                if(tx->issuedTime < TargetTx->issuedTime)
                {
                    cpyTxLegit.push_back(std::make_pair(ComputeWeight(tx),tx));
                }
            }

            if(!cpyTxLegit.empty())
            {
                std::sort(cpyTxLegit.begin(), cpyTxLegit.end(), [](const std::pair<int,pTr_S>& p1, const std::pair<int,pTr_S>& p2){return p1.first > p2.first;});
                RootTx = cpyTxLegit.at(0).second;
                test2 = true;
            }

            else
            {
                cpyTx.clear();
                EV << "Can not find a legit transaction for the chain, retrying later" << std::endl;
                IfAttack = false;
                scheduleAt(simTime() + exponential(mean), msgIssue);
            }
        }

        if(test1 && test2)
        {
            cpyTxLegit.clear();
            cpyTx.clear();

            auto T_diff = TargetTx->issuedTime - RootTx->issuedTime;
            double meanPC = par("PropComputingPower");
            meanPC = meanPC * mean.dbl();
            int weightChain = int(T_diff.dbl()/meanPC);

            double PropChainTips = par("PropChainTips");
            int ChainLength = (int) weightChain*PropChainTips;
            int NbTipsChain = (int) weightChain*(1 - PropChainTips);

            EV << "Chain weight : " << weightChain << std::endl;

            auto TheChain = getParasiteChain(RootTx, TargetTx->ID, ChainLength, NbTipsChain);

            EV << "Launching a Parasite Chain Attack !" << std::endl;

            for(auto tx : TheChain)
            {
                myTangle.push_back(tx);

                if(!tx->isApproved)
                {
                    myTips.insert({tx->ID,tx});
                }

                for(int i = 0; i < NeighborsNumber; i++)
                {
                    MsgUpdate * MsgU = new MsgUpdate;

                    MsgU->ID = tx->ID;
                    MsgU->issuedBy = tx->issuedBy;

                    for(auto approvedTips : tx->S_approved)
                    {
                        MsgU->S_approved.push_back(approvedTips->ID);
                    }

                    msgUpdate->setContextPointer(MsgU);

                    send(msgUpdate->dup(),"NodeOut",i);
                }
            }

            TheChain.clear();
        }
    }

    else if(msg->getKind() == SplittingAttack)
    {
        delete msg;

        EV << "Starting a splitting attack" << std::endl;

        if(myTips.empty())
        {
            EV << "There are no tips for the splitting attack, retrying later" << std::endl;
            IfAttack = false;
            scheduleAt(simTime() + exponential(mean), msgIssue);
        }

        else
        {
            auto tx1 = createSite(ID + std::to_string(txCount));
            auto tx2 = createSite("-" + ID + std::to_string(txCount));

            branch1.push_back(tx1);
            branch2.push_back(tx2);

            simtime_t rateMB = par("rateMB");
            auto msgIni = new cMessage("Initializating the first tips for both branches",InitializationSplittingAttack);
            scheduleAt(simTime() + 2*rateMB, msgIni);
        }
    }

    else if(msg->getKind() == InitializationSplittingAttack)
    {
        EV << "Preparing the two branches in offline..." << std::endl;

        IfNodesfinished();
        double SizeBrancheProp = par("SizeBrancheProp");

        EV << "Branches size : " << branch1.size() + branch2.size() << std::endl;
        EV << "The threshold to reach : " << myTips.size()*SizeBrancheProp << std::endl;

        if(branch1.size() + branch2.size() >= myTips.size()*SizeBrancheProp && !IfSimFinished)
        {
            delete msg;
            VpTr_S cpyTip;

            for(auto tip : myTips)
            {
               cpyTip.push_back(tip.second);
            }

            std::sort(cpyTip.begin(), cpyTip.end(), [](const pTr_S& tip1, const pTr_S& tip2){return tip1->issuedTime > tip2->issuedTime;});

            auto RootTip = cpyTip[0];
            cpyTip.clear();

            auto tx1 = branch1[0];
            auto tx2 = branch2[0];

            tx1->pathID = RootTip->pathID;
            tx1->pathID.insert(tx1->ID);
            tx1->S_approved.push_back(RootTip);

            tx2->pathID = RootTip->pathID;
            tx2->pathID.insert(tx2->ID);
            tx2->S_approved.push_back(RootTip);

            RootTip->approvedBy.push_back(tx1);
            RootTip->approvedBy.push_back(tx2);
            RootTip->approvedTime = simTime();
            RootTip->isApproved = true;

            VpTr_S temp;
            temp.push_back(RootTip);
            ReconcileTips(temp,myTips);
            temp.clear();

            myDoubleSpendTx.push_back(tx2);

            EV << "Reached ! Sending the first tips for the splitting attack" << std::endl;

            std::vector<VpTr_S> toSend;
            toSend.push_back(branch1);
            toSend.push_back(branch2);

            for(auto vec : toSend)
            {
                for(auto tx : vec)
                {
                    if(!tx->isApproved)
                    {
                        tx->pathID = vec[0]->pathID;
                        tx->pathID.insert(tx->ID);
                        myTips.insert({tx->ID,tx});
                    }

                    myTangle.push_back(tx);

                    for(int i = 0; i < NeighborsNumber; i++)
                    {
                        MsgUpdate * MsgU = new MsgUpdate;

                        MsgU->ID = tx->ID;
                        MsgU->issuedBy = tx->issuedBy;

                        for(auto approvedTips : tx->S_approved)
                        {
                            MsgU->S_approved.push_back(approvedTips->ID);
                        }

                        msgUpdate->setContextPointer(MsgU);

                        send(msgUpdate->dup(),"NodeOut",i);
                    }
                }
            }

            IfAttackSP = true;
            IfIniSP = false;
            toSend.clear();
        }

        else if(!IfSimFinished)
        {
            EV << "Not reached ! Issuing new tips..." << std::endl;
            iniSplittingAttack();
            simtime_t rateMB = par("rateMB");
            scheduleAt(simTime() + 2*rateMB, msg);
        }

        else
        {
            delete msg;

            EV << "The other nodes have finished the simulation, can't initiate the attack" << std::endl;
            std::vector<VpTr_S> toSend;
            toSend.push_back(branch1);
            toSend.push_back(branch2);

            for(auto vec : toSend)
            {
                for(auto tx : vec)
                {
                    delete tx;
                }
            }

            toSend.clear();
        }
    }

    else if(msg->getKind() == MaintainBalance)
    {
        delete msg;

        if(!IfSimFinished)
        {
            IfNodesfinished();

            if(!IfSimFinished)
            {
                countMB--;
                auto tx = MaintainingBalance(whichBranch);

                EV << "Updated the balance of the branches" << std::endl;
                EV << "With transaction : " << tx->ID << std::endl;
                EV << "countMB : " << countMB << std::endl;

                for(int i = 0; i < NeighborsNumber; i++)
                {
                    MsgUpdate * MsgU = new MsgUpdate;

                    MsgU->ID = tx->ID;
                    MsgU->issuedBy = tx->issuedBy;

                    for(auto approvedTips : tx->S_approved)
                    {
                        MsgU->S_approved.push_back(approvedTips->ID);
                    }

                    msgUpdate->setContextPointer(MsgU);

                    send(msgUpdate->dup(),"NodeOut",i);
                }

                if(countMB == 0 && IfScheduleMB)
                {
                    EV << "Checking the balance..." << std::endl;
                    int diff = branch1.size() - branch2.size();

                    IfScheduleMB = false;

                    if(diff != 0)
                    {
                       EV << "The weight of the two branches is not the same : maintaining..." << std::endl;

                       if(diff > 0)
                       {
                           whichBranch = 2;
                       }

                       else
                       {
                           whichBranch = 1;
                       }

                       countMB = std::abs(diff);
                       IfAttackSP = false;

                       EV << "with countMB : " << countMB << std::endl;
                       EV << "with whichBranch : " << whichBranch << std::endl;

                       simtime_t rateMB = par("rateMB");
                       int temp = txCount;

                       for(int i = 0; i < std::abs(diff); i++)
                       {
                           temp++;
                           EV << "Sending " << ID + std::to_string(temp) << " with a delay of " << (i+1)*rateMB << std::endl;
                           auto msgMB = new cMessage("Maintaining the balance between the two branches",MaintainBalance);
                           scheduleAt(simTime() + (i+1)*rateMB, msgMB);
                       }
                    }

                    else
                    {
                       IfAttackSP = true;
                       EV << "The weight of the two branches is the same" << std::endl;
                    }
                }

                else if(countMB == 0)
                {
                    IfAttackSP = true;
                }
            }
        }

        else
        {
            EV << "The other nodes have finished the simulation, can't maintain the balance" << std::endl;
        }
    }

    else if(msg->getKind() == ISSUE)
    {
        if(txCount < txLimit)
        {
            txCount++;
            VpTr_S chosenTips;
            int tipsNb = 0;
            std::string trId;
            bool Ifattack = false;

            if(strcmp(par("AttackID"),ID.c_str()) == 0)
            {
                bool test = IfAttackStage();

                if(!IfAttack &&  test && (par("ParasiteChainAttack") || par("SplittingAttack")))
                {
                    cMessage * msgAttack = nullptr;
                    IfAttack = true;
                    Ifattack = true;

                    if(par("ParasiteChainAttack"))
                    {
                        msgAttack = new cMessage("Building the parasite chain",ParasiteChainAttack);
                    }

                    else
                    {
                        msgAttack = new cMessage("Starting a splitting attack",SplittingAttack);
                    }

                    scheduleAt(simTime(), msgAttack);
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

            if(!Ifattack)
            {
                EV << "Issuing a new transaction" << std::endl;
                EV << "TSA procedure for " << trId << std::endl;

                double WProp = par("WProp");
                int W = static_cast<int>(WProp*myTangle.size());

                if(strcmp(par("TSA"),"IOTA") == 0)
                {
                   chosenTips = IOTA(par("alpha"),W,par("N"));
                   tipsNb = static_cast<int>(chosenTips.size());
                }

                if(strcmp(par("TSA"),"GIOTA") == 0)
                {
                   chosenTips = GIOTA(par("alpha"),W,par("N"));
                   tipsNb = static_cast<int>(chosenTips.size());
                }

                if(strcmp(par("TSA"),"EIOTA") == 0)
                {
                    chosenTips = EIOTA(par("p1"),par("p2"),par("lowAlpha"),par("highAlpha"),W,par("N"));
                    tipsNb = static_cast<int>(chosenTips.size());
                }

                if(chosenTips.empty())
                {
                    EV << "The TSA did not give legit tips to approve : attempting again." << std::endl;
                    txCount--;
                    scheduleAt(simTime() + exponential(mean), msgIssue);
                }

                else
                {
                    EV << "Chosen Tips : ";

                    for(auto tip : chosenTips)
                    {
                        EV << tip->ID << " ";
                    }

                    EV << std::endl;

                    MsgP->ID = trId;
                    MsgP->chosen = chosenTips;
                    msgPoW->setContextPointer(MsgP);

                    EV << "Pow time = " << tipsNb*powTime << std::endl;
                    scheduleAt(simTime() + tipsNb*powTime, msgPoW);
                }
            }
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
               updateTangle(Msg);
               updateBuffer();
               delete Msg;

               if(par("SplittingAttack") && strcmp(par("AttackID"),ID.c_str()) == 0)
               {
                   if(IfAttackSP && !IfSimFinished)
                   {
                       EV << "Received a new transaction, checking the balance..." << std::endl;
                       int diff = branch1.size() - branch2.size();

                       if(diff != 0)
                       {
                           EV << "The weight of the two branches is not the same : maintaining..." << std::endl;

                            if(diff > 0)
                            {
                                whichBranch = 2;
                            }

                            else
                            {
                                whichBranch = 1;
                            }

                            countMB = std::abs(diff);
                            IfAttackSP = false;

                            EV << "with countMB : " << countMB << std::endl;
                            EV << "with whichBranch : " << whichBranch << std::endl;

                            simtime_t rateMB = par("rateMB");
                            int temp = txCount;

                            for(int i = 0; i < std::abs(diff); i++)
                            {
                                temp++;
                                EV << "Sending " << ID + std::to_string(temp) << " with a delay of " << (i+1)*rateMB << std::endl;
                                auto msgMB = new cMessage("Maintaining the balance between the two branches",MaintainBalance);
                                scheduleAt(simTime() + (i+1)*rateMB, msgMB);
                            }
                        }

                        else
                        {
                            IfAttackSP = true;
                            EV << "The weight of the two branches is the same" << std::endl;
                        }
                    }

                   else if(!IfSimFinished && !IfIniSP)
                   {
                       EV << "Currently maintaining the balance, buffering the request..." << std::endl;
                       IfScheduleMB = true;
                   }
               }
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

    if(strcmp(par("AttackID"),ID.c_str()) == 0 && par("SplittingAttack"))
    {
        EV << "Size branch 1 : " << branch1.size() << std::endl;
        EV << "Size branch 2 : " << branch2.size() << std::endl;
        PercentDiffBranch();
    }

    else if(strcmp(par("AttackID"),ID.c_str()) == 0 && par("ParasiteChainAttack"))
    {
        DiffTxChain();
    }

    if(par("ParasiteChainAttack") || par("SplittingAttack"))
    {
        //printChain();
    }

    //printTangle();
    //printTipsLeft();
    //stats();

    DeleteTangle();
    delete msgIssue;
    delete msgPoW;
    delete msgUpdate;
    delete MsgP;
}
