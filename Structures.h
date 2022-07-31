//includes 
#pragma once
#include <omnetpp.h>
#include <random>
#include <omp.h>

using namespace omnetpp;

struct Tx;
using VpTx = std::vector<Tx*>;
enum MessageType{ISSUE,POW,UPDATE,ATTACK,PCA,SPA,MB};

//a transaction
struct Tx
{
    Tx(std::string _ID):ID(_ID) //constructor
    {
        for(int i = 0; i < omp_get_num_procs(); i++)
        {
            isVisited.push_back(false);
            confidence.push_back(0.0);
            countSelected.push_back(0);
        }
    };

    const std::string ID; //the ID of the transaction
    
    std::vector<double> confidence; //confidence (for G-IOTA) (each idx is associated with a thread based on his ID)
    std::vector<int> countSelected; //how much a tip has been selected during a TSA (for G-IOTA) (each idx is associated with a thread based on his ID)

    bool isGenesisBlock = false;
    bool isApproved = false; 
    std::vector<bool> isVisited; //used during the recursion weight compute process (each idx is associated with a thread based on his ID)
    
    VpTx approvedBy; //transactions that have approved this transaction directly
    VpTx approvedTx; //transactions approved by this transaction directly

    simtime_t issuedTime; //simulation time when the transaction was issued
    simtime_t approvedTime; //simulation time when this transaction ceased to be a tip (i.e has been approved)

    std::map<std::string,std::pair<bool,bool>> conflictTx; //stores conflicted transaction approved by this transaction
};

//data to send to others modules to update their tangles
struct dataUpdate
{
    std::string ID;  //the ID of the transaction
    std::vector<std::string> approvedTx; //transactions approved by this transaction
};

/* a rng class to avoid the use of the built in rng functions because, sometimes, 
*  the omnet++ exponential distribution returns a high negative value leading to msgs be sended in the past. 
*  This bug appeared since the introduction of the OpenMP code and I duno why ?
*  The setted seed is unique for each module and each run (check AbstractModule::_initialize())
*/
class randomNumberGenerator
{
    public:
        //constructor
        randomNumberGenerator(int seed, double lambda):eng(rd()), 
                                                       exponential(std::exponential_distribution<double>(lambda)),
                                                       uniformBetweenZeroOne(std::uniform_real_distribution<double>{0.0,1.0})
        {
            eng.seed(seed);
        };

        //exponential distribution with the given mean setted in the construction
        double exp()
        {
            return exponential(eng);
        };

        //uniform discrete distribution with the given interval (int)
        int intUniform(int a, int b)
        {
            return std::uniform_int_distribution<int>{a,b}(eng);
        };

        //uniform discrete distribution in the interval [0.0,1.0) (double)
        double doubleUniformBetweenZeroOne()
        {
            return uniformBetweenZeroOne(eng);
        };

        //uniform discrete distribution with the given interval (double)
        double doubleUniform(double a, double b)
        {
            return std::uniform_real_distribution<double>{a,b}(eng);
        };

    private:
        std::random_device rd; //random device
        std::mt19937 eng; //Mersenne twister engine
        std::exponential_distribution<double> exponential; //STL exponential distribution
        std::uniform_real_distribution<double> uniformBetweenZeroOne; //STL uniform distribution in [0.0,1.0)
};