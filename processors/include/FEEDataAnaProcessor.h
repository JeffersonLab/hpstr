#ifndef __ECALTIMING_ANAPROCESSOR_H__
#define __ECALTIMING_ANAPROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "TSData.h"
#include "VTPData.h"
#include "CalCluster.h"
#include "Track.h"
#include "HistoManager.h"
#include "FEEDataAnaHistos.h"


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

class FEEDataAnaProcessor : public Processor {

public:
	FEEDataAnaProcessor(const std::string& name, Process& process);
    ~FEEDataAnaProcessor();
    virtual bool process(IEvent* ievent);

    virtual void initialize(TTree* tree);

    virtual void finalize();

    virtual void configure(const ParameterSet& parameters);

private:
    //Containers to hold histogrammer info
    FEEDataAnaHistos* histos{nullptr};
    std::string  histCfgFilename_;

    //TODO Change this to be held from HPSEvent
    TTree* tree_{nullptr};
    TBranch* bvtpData_{nullptr};
    TBranch* btsData_{nullptr};
    TBranch* btrks_{nullptr};
    TBranch* becalClusters_{nullptr};

    VTPData* vtpData_{};
    TSData* tsData_{};
    std::vector<Track*>  * trks_{};
    std::vector<CalCluster*> * ecalClusters_{};

    std::string anaName_{"ecalTimingAna"};
    std::string vtpColl_{"VTPBank"};
    std::string tsColl_{"TSBank"};
    std::string trkColl_{"GBLTracks"};
    std::string ecalClusColl_{"EcalClustersCorr"};

    //Debug level
    int debug_{0};

    double beamE_{4.55};

    /*
     * Track-cluster matching
     */

    //Cut functions for X
    TF1 *func_top_topCutX;
    TF1 *func_top_botCutX;

    TF1 *func_bot_topCutX;
    TF1 *func_bot_botCutX;

    //Cut functions for Y
    TF1 *func_top_topCutY;
    TF1 *func_top_botCutY;

    TF1 *func_bot_topCutY;
    TF1 *func_bot_botCutY;


    //Parameters of cut functions for X
    double top_topCutX[2] = {8.08915, 0.9176};
    double top_botCutX[2] = {-17.9579, 0.884725};

    double bot_topCutX[2] = {8.50651, 0.94637};
    double bot_botCutX[2] = {-12.2999, 0.943741};

    //Parameters of cut functions for Y
    double top_topCutY[2] = {4.73733, 0.976192};
    double top_botCutY[2] = {-1.38761, 0.944777};

    double bot_topCutY[2] = {4.53276, 0.9928};
    double bot_botCutY[2] = {-8.09522, 0.927891};
};

#endif
