#ifndef __SVTBL2D_EVIOPROCESSOR_H__
#define __SVTBL2D_EVIOPROCESSOR_H__

//HPSTR
#include "Processor.h"
#include "ModuleMapper.h"
#include "RawSvtHit.h"
#include "Svt2DBlHistos.h"

//EVIO
#include "EvioTool/HPSEvioReader.h"

//ROOT
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
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

class SvtBl2DEvioProcessor : public Processor {

    public:

        SvtBl2DEvioProcessor(const std::string& name, Process& process);

        ~SvtBl2DEvioProcessor();

        virtual void configure(const ParameterSet& parameters);

        virtual void initialize(std::string inFilename, std::string outFilename);

        virtual bool process();

        virtual void initialize(TTree* tree) {};

        virtual bool process(IEvent* event) { return true;};

        virtual void finalize();

    private:

        //Debug Level
        int debug_{0};
        Svt2DBlHistos* svtCondHistos{nullptr};

        //Initialize some containers
        ModuleMapper * mmapper_;
        std::vector<RawSvtHit*> rawSvtHits_;
        
        //configuration parameters
        std::string histCfgFilename_;
        std::string trigFilename_{""};
        std::string inFilename_{""};

        //evio interface 
        HPSEvioReader * etool{nullptr};
        
        int run_number_{0};
        unsigned long trigtime_start_{0};
};


#endif
