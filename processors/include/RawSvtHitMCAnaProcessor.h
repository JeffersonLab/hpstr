#ifndef __RAWSVTHITMC_ANAPROCESSOR_H__
#define __RAWSVTHITMC_ANAPROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "RawSvtHit.h"

//ROOT
#include "RawSvtHitMCHistos.h"
#include "Processor.h"
#include "TClonesArray.h"
#include "TFile.h"

class TTree;


class RawSvtHitMCAnaProcessor : public Processor {

    public:

        RawSvtHitMCAnaProcessor(const std::string& name, Process& process);

        ~RawSvtHitMCAnaProcessor();

        virtual bool process(IEvent* ievent);

        virtual void initialize(TTree* tree);

        virtual void finalize();

        virtual void configure(const ParameterSet& parameters);


    private:

        std::string histCfgFilename_;
        RawSvtHitMCHistos* rawSvtHitHistos_{nullptr};

        std::string               rawSvtHitsColl_{"SVTRawTrackerHits"};
        std::vector<RawSvtHit*> * rawSvtHits_{};
        TBranch*                  brawSvtHits_{nullptr};

        TTree* tree_;

        int debug_{0};

};


#endif
