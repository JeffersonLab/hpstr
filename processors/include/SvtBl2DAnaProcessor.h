#ifndef __SVTBL2D_ANAPROCESSOR_H__
#define __SVTBL2D_ANAPROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "RawSvtHit.h"
#include "ModuleMapper.h"

//ROOT
#include "Svt2DBlHistos.h"
#include "Processor.h"
#include "TClonesArray.h"
#include "TFile.h"

class TTree;


class SvtBl2DAnaProcessor : public Processor {

    public:

        SvtBl2DAnaProcessor(const std::string& name, Process& process);

        ~SvtBl2DAnaProcessor();

        virtual bool process(IEvent* ievent);

        virtual void initialize(TTree* tree);

        virtual void finalize();

        virtual void configure(const ParameterSet& parameters);


    private:

        int Event_number=0;
        Svt2DBlHistos* svtCondHistos{nullptr};

        //Initialize ModuleMapper
        ModuleMapper * mmapper_;

        //configuration parameters
        std::string histCfgFilename_;
        std::string               rawSvtHitsColl_{"SVTRawTrackerHits"};
        std::vector<RawSvtHit*> * rawSvtHits_{};
        TBranch*                 brawSvtHits_{nullptr};
        TTree* tree_;
        int debug_{0};

};


#endif
