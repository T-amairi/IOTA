//includes
#pragma once
#include "AbstractModule.h"

//HonestModule : honest user issuing transactions
class HonestModule : public AbstractModule
{
    public:
        //overridden functions from AbstractModule class called in handleMessage() function
        void caseISSUE() override;
        void casePOW(cMessage* msg) override;
        void caseUPDATE(cMessage* msg) override;

        //overridden functions from cSimpleModule class
        void initialize() override;
        void handleMessage(cMessage * msg) override;
        void finish() override;
};

Define_Module(HonestModule);
