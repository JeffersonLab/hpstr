#ifndef __FEEMC_ANAPROCESSOR_H__
#define __FEEMC_ANAPROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "CalCluster.h"
#include "Track.h"
#include "HistoManager.h"
#include "FEEMCAnaHistos.h"


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

class FEEMCAnaProcessor : public Processor {

public:
	FEEMCAnaProcessor(const std::string& name, Process& process);
    ~FEEMCAnaProcessor();
    virtual bool process(IEvent* ievent);

    virtual void initialize(TTree* tree);

    virtual void finalize();

    virtual void configure(const ParameterSet& parameters);

private:
    //Containers to hold histogrammer info
    FEEMCAnaHistos* histos{nullptr};
    std::string  histCfgFilename_;

    //TODO Change this to be held from HPSEvent
    TTree* tree_{nullptr};
    TBranch* bgtpClusters_{nullptr};
    TBranch* btrks_{nullptr};
    TBranch* becalClusters_{nullptr};

    std::vector<CalCluster*> * gtpClusters_{};
    std::vector<Track*>  * trks_{};
    std::vector<CalCluster*> * ecalClusters_{};

    std::string anaName_{"ecalTimingAna"};
    std::string gtpClusColl_{"RecoEcalClustersGTP"};
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
    double top_topCutX[2] = {22.4311, 0.865347};
    double top_botCutX[2] = {-20.9517, 0.889685};

    double bot_topCutX[2] = {23.5512, 0.867873};
    double bot_botCutX[2] = {-21.6512, 0.888276};

    //Parameters of cut functions for Y
    double top_topCutY[2] = {7.1885, 0.900118};
    double top_botCutY[2] = {-5.84294, 0.903104};

    double bot_topCutY[2] = {4.91401, 0.88867};
    double bot_botCutY[2] = {-6.39967, 0.910099};
};

#endif
