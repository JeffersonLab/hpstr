#ifndef __FITBL_PROCESSOR_H__
#define __FITBL_PROCESSOR_H__
#include <iostream>

//ROOT
#include "Processor.h"
#include "TFile.h"
#include "TTree.h"

//HPSTR
#include "IEvent.h"


class SvtBlFitHistoProcessor : public Processor {

    public: 

        SvtBlFitHistoProcessor(const std::string& name, Process& process);
        
        ~SvtBlFitHistoProcessor();

        virtual void configure(const ParameterSet& parameters);
        virtual void initialize(std::string inFilename, std::string outFilename);
        virtual bool process();
        virtual void initialize() {};
        virtual void finalize();

        virtual bool process(IEvent* ievent) {};
        virtual void initialize(TTree* tree) {};

    private:

        TFile* inf_{nullptr};


};

#endif
