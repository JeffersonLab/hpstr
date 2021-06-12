#ifndef __READOUTDIAGNOSTICS_ANAPROCESSOR_H__
#define __READOUTDIAGNOSTICS_ANAPROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "CalCluster.h"
#include "HistoManager.h"
#include "ReadoutDiagnosticsAnaHistos.h"


//ROOT
#include "Processor.h"
#include "TFile.h"
#include "TTree.h"
#include "TRefArray.h"
#include "TBranch.h"
#include "TVector3.h"
#include "TLorentzVector.h"
#include "TF1.h"

//C++
#include <memory>

class ReadoutDiagnosticsAnaProcessor : public Processor {

public:
	ReadoutDiagnosticsAnaProcessor(const std::string& name, Process& process);
    ~ReadoutDiagnosticsAnaProcessor();
    virtual bool process(IEvent* ievent);

    virtual void initialize(TTree* tree);

    virtual void finalize();

    virtual void configure(const ParameterSet& parameters);

private:
    //Containers to hold histogrammer info
    ReadoutDiagnosticsAnaHistos* histos{nullptr};
    std::string  histCfgFilename_;

    //TODO Change this to be held from HPSEvent
    TTree* tree_{nullptr};
    TBranch* becalClusters_{nullptr};

    std::vector<CalCluster*> * ecalClusters_{};

    std::string anaName_{"readoutDiagnosticsAnaProcessor"};
    std::string ecalClusColl_{"EcalClustersCorr"};

    //Debug level
    int debug_{0};

    double beamE_{4.55};
};

#endif
