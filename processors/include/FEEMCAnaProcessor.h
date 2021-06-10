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
    FEEMCAnaHistos* histos_prescale{nullptr};
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

    /* hps_v6_FEE
    #                     prescale region
    #                     |   region xmin
    #                     |   |     region xmax
    #                     |   |     |     prescale
    #                     |   |     |     |
    VTP_HPS_FEE_PRESCALE  0   -22   16    0
    VTP_HPS_FEE_PRESCALE  1   -15   -9    1
    VTP_HPS_FEE_PRESCALE  2    -8   -7    10
    VTP_HPS_FEE_PRESCALE  3    -6   -4    100
    VTP_HPS_FEE_PRESCALE  4    -3   -2    10
    VTP_HPS_FEE_PRESCALE  5    -1    6    1
    VTP_HPS_FEE_PRESCALE  6     7   23    0
     */
    int regionsLeft[7] = {-22, -15, -8, -6, -3, -1, 7};
    int regionsRight[7] = {-16, -9, -7, -4, -2, 6, 23};
    int preScale[7] = {0, 1, 10, 100, 10, 1, 0};

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
    double top_topCutX[2] = {11.9832, 0.909758};
    double top_botCutX[2] = {-8.25902, 0.925639};

    double bot_topCutX[2] = {11.4547, 0.925082};
    double bot_botCutX[2] = {-7.68077, 0.926624};

    //Parameters of cut functions for Y
    double top_topCutY[2] = {5.82916, 0.940971};
    double top_botCutY[2] = {-0.960051, 0.922741};

    double bot_topCutY[2] = {0.168838, 0.944334};
    double bot_botCutY[2] = {-9.5317, 0.916178};
};

#endif
