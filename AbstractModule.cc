#include "AbstractModule.h"

Tx* AbstractModule::createTx() 
{
    auto newTx = new Tx(ID + std::to_string(txCount));
    newTx->issuedTime = simTime();
    txCount++;
    return newTx;
}

void AbstractModule::createGenBlock() 
{
    auto GenBlock = new Tx("Genesis");
    GenBlock->issuedTime = simTime();
    GenBlock->isGenesisBlock = true;

    myTangle.push_back(GenBlock);
    myTips.push_back(GenBlock);
}

int AbstractModule::getTxCount() const
{
    return txCount;
}

int AbstractModule::getTxLimit() const
{
    return txLimit;
}

void AbstractModule::setTxLimit(int newLimit)
{
    txLimit = newLimit;
}

void AbstractModule::printTangle() const
{
    std::fstream file;
    std::string path = "./data/tracking/Tangle" + ID + ".csv";
    remove(path.c_str());
    file.open(path,std::ios::app);

    for(auto const tx : myTangle)
    {
       file << tx->ID << ";";

        for(size_t j = 0; j < tx->approvedBy.size(); j++)
        {
            auto temp = tx->approvedBy[j];

            if(j == tx->approvedBy.size() - 1)
            {
                file << temp->ID;
            }

            else
            {
                file << temp->ID << ",";
            }
        }

        file << "\n";
    }

    file.close();
}

void AbstractModule::printTipsLeft(bool ifDeleteFile) const
{
    std::fstream file;
    std::string path = "./data/tracking/TipsNumber" + ID + ".csv";
    if(ifDeleteFile) remove(path.c_str());
    file.open(path,std::ios::app);
    file << myTips.size() << "\n";
    file.close();
}

void AbstractModule::printStats(bool ifDeleteFile) const
{
    std::fstream file;
    std::string path = "./data/tracking/Stats" + ID + ".csv";
    if(ifDeleteFile) remove(path.c_str());
    file.open(path,std::ios::app);
    file << getParentModule()->getName() << ";" << txCount << ";" << myTangle.size() << ";" << myTips.size() << ";\n";
    file.close();
}

VpTx AbstractModule::IOTA(double alphaVal, int W, int N)
{
    if(myTips.size() == 1)
    {
        VpTx tipsToApprove;
        if(isLegitTip(myTips[0])) tipsToApprove.push_back(myTips[0]);
        return tipsToApprove;
    }

    std::vector<std::tuple<Tx*,int,int>> selectedTips;
    int walkTime;
    Tx* tip;

    for(int i = 0; i < N; i++)
    {
        int w = intuniform(W,2*W);
        Tx* startTx = getWalkStart(w);

        alphaVal ? tip = weightedRandomWalk(startTx,alphaVal,walkTime) : tip = randomWalk(startTx,walkTime);
        auto it = selectedTips.begin();

        for(; it != selectedTips.end(); it++)
        {
            if(std::get<0>(*it)->ID == tip->ID)
            {
                std::get<1>(*it)++;
                break;
            }
        }

        if(it == selectedTips.end() && isLegitTip(tip))
        {
            selectedTips.push_back(std::make_tuple(tip,1,walkTime));
        }
    }

    std::sort(selectedTips.begin(),selectedTips.end(),[](const std::tuple<Tx*,int,int>& tup1,const std::tuple<Tx*,int,int>& tup2){return std::get<1>(tup1) > std::get<1>(tup2);});

    if(selectedTips.size() == 1)
    {
        VpTx chosenTips = {std::get<0>(selectedTips[0])};
        return chosenTips;
    }

    VpTx chosenTips = {std::get<0>(selectedTips[0])};

    for(size_t i = 1; i < selectedTips.size(); i++)
    {
        auto tip = std::get<0>((selectedTips[i]));

        if(!ifConflictedTips(chosenTips[0],tip))
        {
            chosenTips.push_back(tip);
            break;
        }
    }

    return chosenTips;
}

VpTx AbstractModule::GIOTA(double alphaVal, int W, int N)
{
    for(auto tx : myTangle)
    {
        tx->confidence = 0.0;
        tx->countSelected = 0;
    }

    auto chosenTips = IOTA(alphaVal,W,N);

    if(chosenTips.size() == 1)
    {
        return chosenTips;
    }

    for(auto tip : myTips)
    {
        if(tip->countSelected && isLegitTip(tip))
        {
           for(auto tx : tip->approvedTx)
            {
               computeConfidence(tx,double(tip->countSelected/N));
            }
        }
    }

    std::vector<std::pair<double,Tx*>> avgConfTips;

    for(auto tip : myTips)
    {
        double avg = 0.0;

        for(auto tx : tip->approvedTx)
        {
            getAvgConf(tx,avg);
        }

        avgConfTips.push_back(std::make_pair(avg,tip));
    }

    std::sort(avgConfTips.begin(), avgConfTips.end(),[](const std::pair<double,Tx*> &tip1, const std::pair<double,Tx*> &tip2){return tip1.first < tip2.first;});

    for(auto pair : avgConfTips)
    {
        auto tip = pair.second;

        if(tip->ID != chosenTips[0]->ID && tip->ID != chosenTips[1]->ID)
        {
            if(!ifConflictedTips(chosenTips[0],tip) && !ifConflictedTips(chosenTips[1],tip))
            {
                chosenTips.push_back(tip);
                break;
            }
        }
    }

    return chosenTips;
}

VpTx AbstractModule::EIOTA(double p1, double p2, double lowAlpha, double highAlpha, int W, int N)
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

VpTx AbstractModule::getTipsTSA()
{
    EV << "TSA procedure for the next transaction to be issued: " << ID + std::to_string(txCount) << "\n";

    double WProp = par("WProp");
    int W = std::round(WProp * myTangle.size());

    if(strcmp(par("TSA"),"GIOTA") == 0)
    {
        return GIOTA(par("alpha"),W,par("N"));
    }

    else if(strcmp(par("TSA"),"EIOTA") == 0)
    {
        return EIOTA(par("p1"),par("p2"),par("lowAlpha"),par("highAlpha"),W,par("N"));
    }

    return IOTA(par("alpha"),W,par("N"));
}

Tx* AbstractModule::weightedRandomWalk(Tx* startTx, double alphaVal, int &walkTime)
{
    walkTime = 0;
    
    while(startTx->isApproved)
    {
        walkTime++;
        auto currentView = startTx->approvedBy;

        if(currentView.size() == 0)
        {
            break;
        }

        if(currentView.size() == 1)
        {
            startTx = currentView.front();
        }

        else
        {
            std::vector<int> txWeight;
            int startWeight = computeWeight(startTx);
            std::vector<double> p;
            double sumExpo = 0.0;
            
            for(auto tx : currentView)
            {
                int weight = computeWeight(tx);
                sumExpo += double(exp(double(-alphaVal*(startWeight - weight))));
                txWeight.push_back(weight);
            }

            for(const auto weight : txWeight)
            {
               auto prob = double(exp(double(-alphaVal*(startWeight - weight))));
               prob = prob/sumExpo;
               p.push_back(prob);
            }

            auto p1 = p;
            std::rotate(p1.rbegin(), p1.rbegin() + 1, p1.rend());
            p1[0] = 0.0;

            std::vector<double> cs;
            cs.resize(p.size());
            std::vector<double> cs1;
            cs1.resize(p1.size());

            std::partial_sum(p.begin(),p.end(),cs.begin());
            std::partial_sum(p1.begin(),p1.end(),cs1.begin());

            int nextIndex = 0;
            double probWalkChoice = uniform(0.0,1.0);

            for(int k = 0; k < static_cast<int>(cs.size()); k++)
            {
                if(probWalkChoice > cs1[k] && probWalkChoice < cs[k])
                {
                    nextIndex = k;
                    break;
                }
            }

            startTx = currentView[nextIndex];
        }
    }

    startTx->countSelected++;
    return startTx;
}

Tx* AbstractModule::randomWalk(Tx* startTx, int &walkTime)
{
    walkTime = 0;
    
    while(startTx->isApproved)
    {
        walkTime++;
        auto currentView = startTx->approvedBy;

        if(currentView.size() == 0)
        {
            break;
        }

        if(currentView.size() == 1)
        {
            startTx = currentView.front();
        }

        else
        {   
            int nextIndex = intuniform(0,currentView.size() - 1);
            startTx = currentView[nextIndex];
        }
    }

    startTx->countSelected++;
    return startTx;
}

Tx* AbstractModule::getWalkStart(int backTrackDist) const
{
    int randomIndex = intuniform(0,myTips.size() - 1);
    auto it(myTips.begin());

    if(myTips.size() > 1) std::advance(it,randomIndex);
    
    auto startTip = *(it);

    while(!startTip->isGenesisBlock && backTrackDist > 0)
    {
        randomIndex = intuniform(0,startTip->approvedTx.size() - 1);
        startTip = startTip->approvedTx[randomIndex];
        backTrackDist--;
    }

    return startTip;
}

void AbstractModule::unionConflictTx(Tx* theSource, Tx* toUpdate)
{
    if(toUpdate->conflictTx.empty())
    {
        toUpdate->conflictTx = theSource->conflictTx;
        return; 
    }

    for(const auto key : theSource->conflictTx)
    {
        auto it = toUpdate->conflictTx.find(key.first);

        if(it == toUpdate->conflictTx.end()) 
        {
            toUpdate->conflictTx[key.first] = key.second;
        }

        else
        {
            if(key.second.first)
            {
                (*it).second.first = true;
            }

            if(key.second.second)
            {
                (*it).second.second = true;
            }    
        }
    }
}

bool AbstractModule::isLegitTip(Tx* tip)
{
    for(const auto key : tip->conflictTx)
    {
        if(key.second.first && key.second.second)
        {
            return false;
        }
    }

    return true;
}

bool AbstractModule::ifConflictedTips(Tx* tip1, Tx* tip2)
{
    std::map<std::string,std::pair<bool,bool>> outerMap;
    std::map<std::string,std::pair<bool,bool>> innerMap;

    if(tip1->conflictTx.size() >= tip2->conflictTx.size())
    {
        outerMap = tip2->conflictTx;
        innerMap = tip1->conflictTx;
    }

    else
    {
        outerMap = tip1->conflictTx;
        innerMap = tip2->conflictTx;
    }

    for(const auto keyOuter : outerMap)
    {
        auto it = innerMap.find(keyOuter.first);

        if(it != innerMap.end())
        {
            if((*it).second.first && keyOuter.second.second)
            {
                return true;
            }

            else if((*it).second.second && keyOuter.second.first)
            {
                return true;
            }
        }
    }

    return false;
}

void AbstractModule::updateConflictTx(Tx* startTx)
{
    VpTx visitedTx;
    
   _updateconflictTx(startTx,visitedTx,startTx->ID);

   for(auto tx : visitedTx)
   {
       tx->isVisited = false;
   }

   startTx->conflictTx["-" + startTx->ID] = std::make_pair(true,false);
   startTx->isVisited = false;
}

void AbstractModule::_updateconflictTx(Tx* currentTx, VpTx& visitedTx, std::string conflictID)
{
    if(currentTx->isVisited) return;

    currentTx->isVisited = true;
    visitedTx.push_back(currentTx);

    auto it = currentTx->conflictTx.find("-" + conflictID);
    if(it == currentTx->conflictTx.end()) currentTx->conflictTx["-" + conflictID] = std::make_pair(true,false);
    else (*it).second.first = true;
    
    for(auto tx : currentTx->approvedBy)
    {
        if(!tx->isVisited)
        {
            _updateconflictTx(tx,visitedTx,conflictID);
        }
    }
}

bool AbstractModule::isApp(Tx* startTx, std::string idToCheck)
{
    VpTx visitedTx;
    bool res = false;

   _isapp(startTx,visitedTx,idToCheck,res);

   for(auto tx : visitedTx)
   {
       tx->isVisited = false;
   }

   startTx->isVisited = false;

   return res;
}

void AbstractModule::_isapp(Tx* currentTx, VpTx& visitedTx, std::string idToCheck, bool& res)
{
    if(currentTx->isVisited) return;

    currentTx->isVisited = true;
    visitedTx.push_back(currentTx);

    if(currentTx->ID == idToCheck)
    {
        res = true;
        return;
    }

    for(auto tx : currentTx->approvedTx)
    {
        if(!tx->isVisited)
        {
            _isapp(tx,visitedTx,idToCheck,res);
        }
    }
}

int AbstractModule::computeWeight(Tx* tx)
{
    VpTx visitedTx;
    int weight = _computeweight(tx,visitedTx);

    for(auto tx : visitedTx)
    {
        tx->isVisited = false;
    }

    tx->isVisited = false;
    return weight + 1;
}

int AbstractModule::_computeweight(Tx* currentTx, VpTx& visitedTx)
{
    if(currentTx->isVisited)
    {
        return 0;
    }

    else if(currentTx->approvedBy.size() == 0)
    {
        currentTx->isVisited = true;
        visitedTx.push_back(currentTx);
        return 0;
    }

    currentTx->isVisited = true;
    visitedTx.push_back(currentTx);
    int weight = 0;

    for(auto tx : currentTx->approvedBy)
    {
        if(!tx->isVisited)
        {
            weight += 1 + _computeweight(tx,visitedTx);
        }
    }

    return weight;
}

void AbstractModule::computeConfidence(Tx* startTx, double conf)
{
    VpTx visitedTx;
    int dist = par("confDistance");
    _computeconfidence(startTx,visitedTx,dist,conf);

    for(auto tx : visitedTx)
    {
        tx->isVisited = false;
    }

    startTx->isVisited = false;
}

void AbstractModule::_computeconfidence(Tx* currentTx, VpTx& visitedTx, int& distance, double conf)
{
    if(!currentTx->isVisited && distance > 0)
    {
        currentTx->isVisited = true;
        currentTx->confidence += conf;
        visitedTx.push_back(currentTx);
        distance--;

        for(auto tx : currentTx->approvedTx)
        {
            if(!tx->isVisited)
            {
                _computeconfidence(tx,visitedTx,distance,conf);
            }
        }
    }
}

void AbstractModule::getAvgConf(Tx* startTx, double& avg)
{
    VpTx visitedTx;
    int dist = par("confDistance");
    _getavgconf(startTx,visitedTx,dist,avg);

    for(auto tx : visitedTx)
    {
        tx->isVisited = false;
    }

    startTx->isVisited = false;
}

void AbstractModule::_getavgconf(Tx* currentTx, VpTx& visitedTx, int& distance, double& avg)
{
    if(!currentTx->isVisited && distance > 0)
    {
        currentTx->isVisited = true;
        visitedTx.push_back(currentTx);
        avg += currentTx->confidence;
        distance--;

        for(auto tx : currentTx->approvedTx)
        {
            if(!tx->isVisited)
            {
                _getavgconf(tx,visitedTx,distance,avg);
            }
        }
    }
}

bool AbstractModule::isPresent(std::string txID) const
{
    auto it = std::find_if(myTangle.rbegin(), myTangle.rend(), [txID](const Tx* tx) {return tx->ID == txID;});
    return it != myTangle.rend();
}

Tx* AbstractModule::attachTx(simtime_t attachTime, VpTx chosenTips)
{
    auto newTx = createTx();

    EV << "Issuing a new transaction : " << newTx->ID << "\n";

    for(auto tip : chosenTips)
    {
        unionConflictTx(tip,newTx);
        tip->approvedBy.push_back(newTx);

        if(!(tip->isApproved))
        {
            tip->approvedTime = attachTime;
            tip->isApproved = true;
            updateMyTips(tip->ID);
        }
    }

    newTx->approvedTx = chosenTips;
    myTangle.push_back(newTx);
    myTips.push_back(newTx);
    return newTx;
}

Tx* AbstractModule::updateTangle(const dataUpdate* data)
{
    EV << "Updating the Tangle\n";

    auto newTx = new Tx(data->ID);
    newTx->issuedTime = simTime();
    
    if(newTx->ID[0] == '-')
    {
        newTx->conflictTx[newTx->ID] = std::make_pair(false,true);
        myBuffer.push_back(std::string(newTx->ID.begin() + 1, newTx->ID.end()));
    }

    for(const auto tipID : data->approvedTx)
    {
        auto it = std::find_if(myTangle.rbegin(), myTangle.rend(), [tipID](const Tx* tx) {return tx->ID == tipID;});
        
        if(it != myTangle.rend())
        {
            unionConflictTx(*it,newTx);
            newTx->approvedTx.push_back(*it);
            (*it)->approvedBy.push_back(newTx);

            if(!((*it)->isApproved))
            {
                (*it)->approvedTime = simTime();
                (*it)->isApproved = true;
                updateMyTips((*it)->ID);
            }
        }
    }

    myTangle.push_back(newTx);
    myTips.push_back(newTx);
    
    return newTx;
} 

void AbstractModule::updateMyTips(std::string tipID)
{
    auto it = std::find_if(myTips.begin(), myTips.end(), [tipID](const Tx* tx) {return tx->ID == tipID;});

    if(it != myTips.end())
    {
        myTips.erase(it);
    }
}

void AbstractModule::updateMyBuffer()
{
    for(auto it = myBuffer.begin(); it != myBuffer.end();)
    {
        auto itTx = std::find_if(myTangle.begin(), myTangle.end(), [it](const Tx* tx) {return tx->ID == *it;});

        if(itTx != myTangle.end())
        {
            it = myBuffer.erase(it);
            updateConflictTx(*itTx);
        }

        else
        {
            it++;
        } 
    }
}

void AbstractModule::spreadTx(cModule* senderModule, dataUpdate* data) 
{
    EV << " Sending " << data->ID << " to all nodes\n";

    for(int i = 0; i < gateSize("out"); i++)
    {
        cGate *g = gate("out",i);

        if(!g->pathContains(senderModule))
        {
            auto copyData = new dataUpdate;
            copyData->ID = data->ID;
            copyData->approvedTx = data->approvedTx;

            msgUpdate->setContextPointer(copyData);
            send(msgUpdate->dup(),"out",i);
        }
    }
}

void AbstractModule::broadcastTx(const Tx* newTx)
{
    auto data = new dataUpdate;
    data->ID = newTx->ID;

    for(const auto approvedTips : newTx->approvedTx)
    {
        data->approvedTx.push_back(approvedTips->ID);
    }

    for(int i = 0; i < gateSize("out"); i++)
    {
        auto copyData = new dataUpdate;
        copyData->ID = data->ID;
        copyData->approvedTx = data->approvedTx;

        msgUpdate->setContextPointer(copyData);
        send(msgUpdate->dup(),"out",i);
    }

    delete data;
}

void AbstractModule::deleteTangle()
{
    for(auto tx : myTangle)
    {
        delete tx;
    }
}

void AbstractModule::_initialize()
{
    ID = "[" + std::to_string(getId() - 2) + "]";
    rateMean = par("rateMean");
    powTime = par("powTime");
    txLimit = par("transactionLimit");

    createGenBlock();

    msgIssue = new cMessage("Issuing a new transaction",ISSUE);
    msgPoW = new cMessage("PoW time",POW);
    msgUpdate = new cMessage("Broadcasting a new transaction",UPDATE);
}

void AbstractModule::_finish(bool exportTangle, std::pair<bool,bool> exportTipsNumber)
{
    EV << "Exporting data to csv files and clearing memory\n";

    if(exportTangle)
    {
        printTangle();
    }

    if(exportTipsNumber.first)
    {
        printTipsLeft(exportTipsNumber.second);
    }

    deleteTangle();
    delete msgIssue;
    delete msgPoW;
    delete msgUpdate;
}
