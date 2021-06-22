//includes
#include "iota.h"
#include <iostream>
#include <random>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <stdio.h>
#include <regex>
#include <string>
#include <utility>
#include <numeric>
#include <fstream>
#include <cmath>
#include <functional>

std::mt19937& RNG::getRandGen()
{
    return tipSelectGen;
}

RNG* Node::getRNGPtr() const
{
    return RNGPtr;
}

void Node::setRNGPtr(RNG* rng)
{
    RNGPtr = rng;
}

bool Node::ifIssue(double prob)
{
    //std::random_device rd;
    //std::mt19937 tm(rd());
    std::uniform_real_distribution<double> ud(0.0,1.0);

    /*std::fstream file;
    std::string path = "./data/Tracking/TrackerTangle.txt";
    file.open(path,std::ios::app);*/

    double test = 0.1; //ud(getRNGPtr()->getRandGen());

    //file << test << std::endl;

    if(test <= prob)
    {
        return true;
    }

    //file.close();
    return false;
}

pTr_S Node::createSite(std::string ID)
{
    pTr_S newTr = new Site;

    newTr->ID =  ID;
    newTr->issuedBy = this;
    newTr->issuedTime = omnetpp::simTime();

    return newTr;
}

pTr_S Node::createGenBlock()
{
    pTr_S GenBlock = Node::createSite("Genesis");
    GenBlock->isGenesisBlock = true;

    return GenBlock;
}

void Node::DeleteTangle(Node &myNode, VpTr_S &myTangle)
{
    myNode.ifDeleted = true;

    for(auto site : myTangle)
    {
        if(site->issuedBy->ID == myNode.ID)
        {
            delete site;
        }
    }
}

void Node::printTangle(VpTr_S myTangle, pTr_S Gen)
{
    std::fstream file;
    std::string path = "./data/Tracking/TrackerTangle.txt";
    file.open(path,std::ios::app);

    omnetpp::simtime_t sec = omnetpp::simTime();

    file << sec << ";";
    file <<"Genesis"<< ";";

    for(long unsigned int i = 0; i < Gen->approvedBy.size(); i++)
    {
        if(i == Gen->approvedBy.size() - 1)
        {
            file << Gen->approvedBy[i]->ID;
        }

        else
        {
            file << Gen->approvedBy[i]->ID << ",";
        }
    }

    file << std::endl;

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

    file << std::endl;

    file.close();
}

void Node::printTipsLeft(int numberTips)
{
    std::fstream file;
    std::string path = "./data/log/NumberTips.txt";
    file.open(path,std::ios::app);

    file << numberTips << std::endl;

    file.close();
    return;
}

std::map<int,pTr_S> Node::giveTips()
{
     return myTips;
}

int Node::_computeWeight(VpTr_S& visited, pTr_S& current, omnetpp::simtime_t timeStamp)
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

    for(int i = 0; i < current->approvedBy.size(); ++i)
    {
        if(!current->approvedBy.at(i)->isVisited)
        {
                weight += 1 + _computeWeight(visited, current->approvedBy.at(i), timeStamp);
        }
    }

    return weight;
}

int Node::ComputeWeight(pTr_S tr, omnetpp::simtime_t timeStamp)
{
    VpTr_S visited;
    int weight = _computeWeight(visited, tr, timeStamp);

    for(int i = 0; i < visited.size(); ++i)
    {
        for(int j = 0; j < visited.at(i)->approvedBy.size(); ++j)
        {
            visited.at(i)->approvedBy.at(j)->isVisited = false;
        }

    }

    tr->isVisited = false;
    return weight + 1;
}

pTr_S Node::getWalkStart(std::map<int,pTr_S>& tips, int backTrackDist)
{
    std::random_device rd;
    std::mt19937 tm(rd());
    std::uniform_int_distribution<int> tipDist(0,tips.size() - 1);

    int iterAdvances = tipDist(tm);

    assert(iterAdvances < tips.size());

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
        std::uniform_int_distribution<int> choice(0,current->S_approved.size() - 1);
        approvesIndex = choice(tm);
        assert(approvesIndex < current->S_approved.size());
        current = current->S_approved.at(approvesIndex);
        --count;
    }

    return current;
}

bool Node::isRelativeTip(pTr_S& toCheck, std::map<int, pTr_S>& tips)
{
    std::map<int,pTr_S>::iterator it;

    for(it = tips.begin(); it != tips.end(); it++)
    {
        if(it->second->ID == toCheck->ID)
        {
            break;
        }
    }

    if(it == tips.end())
    {
        return false;
    }

    else
    {
        return true;
    }
}

void Node::filterView(VpTr_S& view, omnetpp::simtime_t timeStamp)
{
    std::vector<int> removeIndexes;

    for(int i = 0; i < view.size(); ++i)
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

void Node::ReconcileTips(const VpTr_S& removeTips, std::map<int,pTr_S>& myTips)
{
    std::map<int,pTr_S>::iterator it;

    for(auto& tipSelected : removeTips)
    {
       for(it = myTips.begin(); it != myTips.end(); it++)
       {
           if(tipSelected->ID == it->second->ID)
           {
               break;
           }
       }

        if(it != myTips.end())
        {
            myTips.erase(it);
        }
    }
}

pTr_S Node::attach(Node& myNode, std::string ID, omnetpp::simtime_t attachTime, VpTr_S& chosen)
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
     ReconcileTips(chosen,myNode.myTips);

     myNode.myTangle.push_back(new_tips);
     myNode.myTips.insert({myNode.myTips.size() + 1,new_tips});

     return new_tips;
}

//TODO: case alpha = 0.0 (dont compute weight)
pTr_S Node::RandomWalk(pTr_S start, double alphaVal, std::map<int, pTr_S>& tips, omnetpp::simtime_t timeStamp, int &walk_time)
{
    std::random_device rd;
    std::mt19937 tm(rd());

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
            std::uniform_real_distribution<double> walkChoice(0.0,1.0);

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
            double probWalkChoice =  walkChoice(tm);

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

VpTr_S Node::IOTA(double alphaVal, std::map<int, pTr_S>& tips, omnetpp::simtime_t timeStamp, int W, int N)
{
    std::random_device rd;
    std::mt19937 tm(rd());
    std::uniform_int_distribution<int> backTrackDist(W,2*W);

    VpTr_S start_sites;
    pTr_S temp_site;
    int backTD;

    for(int i = 0; i < N; i++)
    {
        backTD = backTrackDist(tm);
        temp_site = getWalkStart(tips,backTD);
        start_sites.push_back(temp_site);
    }

    VpTr_S selected_tips;
    std::vector<std::pair<int,int>> walk_total;
    int walk_time;

    for(int i = 0; i < N; i++)
    {
        if(i != N-1)
        {
            selected_tips.push_back(RandomWalk(start_sites[i],alphaVal,tips,timeStamp,walk_time));
            walk_total.push_back(std::make_pair(i,walk_time));
        }

        else
        {
            selected_tips.push_back(RandomWalk(start_sites[i],alphaVal,tips,timeStamp,walk_time));
            walk_total.push_back(std::make_pair(i,walk_time));
        }
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

VpTr_S Node::GIOTA(double alphaVal, std::map<int, pTr_S>& tips, omnetpp::simtime_t timeStamp, int W, int N)
{
   VpTr_S chosenTips = IOTA(alphaVal,tips,timeStamp,W,N);
   VpTr_S copyTips;

   for(auto it = tips.begin(); it != tips.end(); ++it)
   {
       auto tip = it->second;

       for(int j = 0; j < static_cast<int>(chosenTips.size()); j++)
       {
           if(chosenTips[j]->ID != tip->ID)
           {
               copyTips.push_back(tip);
               break;
           }
       }
   }

   if(!copyTips.size())
   {
       return chosenTips;
   }

   std::random_device rd;
   std::mt19937 tm(rd());
   std::uniform_int_distribution<int> ThirdTipsIdx(0,static_cast<int>(copyTips.size()) - 1);

   auto idx = ThirdTipsIdx(tm);
   auto LeftTips = copyTips[idx];

   chosenTips.push_back(LeftTips);
   return chosenTips;
}

VpTr_S Node::EIOTA(double p1, double p2, std::map<int, pTr_S>& tips, omnetpp::simtime_t timeStamp, int W, int N)
{
    std::random_device rd;
    std::mt19937 tm(rd());
    std::uniform_real_distribution<> rGen(0,1);

    double lowAlpha = 0.1;
    double highAlpha = 5.0;
    auto r = rGen(tm);
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

void Node::updateTangle(pTr_S toAdd, omnetpp::simtime_t attachTime)
{
    this->myTips.insert({this->myTips.size() + 1,toAdd});
    ReconcileTips(toAdd->S_approved,this->myTips);

    for (auto& tipSelected : toAdd->S_approved)
    {
        for(auto& tr : this->myTangle)
        {
            if(tr->ID == tipSelected->ID)
            {
                tr->approvedBy.push_back(toAdd);

                if(!(tr->isApproved))
                {
                    tr->approvedTime = attachTime;
                    tr->isApproved = true;
                }
            }
        }
    }
}
