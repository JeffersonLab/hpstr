#ifndef __TS_ANAPROCESSOR_H__
#define __TS_ANAPROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "TSData.h"
#include "HistoManager.h"
#include "TSAnaHistos.h"


//ROOT
#include "Processor.h"
#include "TFile.h"
#include "TTree.h"
#include "TRefArray.h"
#include "TBranch.h"
#include "TVector3.h"
#include "TLorentzVector.h"

//C++
#include <memory>

class TSAnaProcessor : public Processor {

public:
	TSAnaProcessor(const std::string& name, Process& process);
    ~TSAnaProcessor();
    virtual bool process(IEvent* ievent);

    virtual void initialize(TTree* tree);

    virtual void finalize();

    virtual void configure(const ParameterSet& parameters);

private:
    //Containers to hold histogrammer info
    TSAnaHistos* histos{nullptr};
    std::string  histCfgFilename_;

    //TODO Change this to be held from HPSEvent
    TTree* tree_{nullptr};
    TBranch* btsData_{nullptr};

    TSData* tsData_{};

    std::string anaName_{"vtxAna"};
    std::string tsColl_{"TSBank"};

    //Debug level
    int debug_{0};

    double beamE_{4.55};
};

#endif
