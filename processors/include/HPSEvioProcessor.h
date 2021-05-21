#ifndef __HPS_EVIOPROCESSOR_H__
#define __HPS_EVIOPROCESSOR_H__

//HPSTR
#include "Processor.h"
//EVIO
#include "EvioTool/HPSEvioReader.h"

//ROOT
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TAxis.h"

//CPLUSPLUS
#include <map>
#include <vector>
#include <memory>
#include <ratio>
#include <chrono>
#include <limits.h>
#include <iostream>
#include <string>

class HPSEvioProcessor : public Processor {

    public:

        HPSEvioProcessor(const std::string& name, Process& process);

        ~HPSEvioProcessor();

        virtual void configure(const ParameterSet& parameters);

        virtual void initialize(std::string inFilename, std::string outFilename);

        virtual bool process();

        virtual void initialize(TTree* tree) {};

        virtual bool process(IEvent* event) { return true;};

        virtual void finalize();

    private:

        //Debug Level
        int debug_{0};
        
        //configuration parameters
        std::string trigFilename_{""};

        //evio interface 
        HPSEvioReader * etool{nullptr};
        
        int run_number_{0};
        unsigned long trigtime_start_{0};
};


#endif
