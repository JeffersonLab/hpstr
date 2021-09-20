#ifndef __TRIGGERVALIDATION_ANAPROCESSOR_H__
#define __TRIGGERVALIDATION_ANAPROCESSOR_H__

#define __WITHSVT__

//HPSTR
#include "HpsEvent.h"
#include "TSData.h"
#include "VTPData.h"
#include "CalCluster.h"
#include "HodoHit.h"
#include "HodoCluster.h"
#include "Track.h"
#include "HistoManager.h"
#include "TriggerValidationAnaHistos.h"
#include "Processor.h"

//ROOT
#include "TFile.h"
#include "TTree.h"
#include "TRefArray.h"
#include "TBranch.h"
#include "TVector3.h"
#include "TF1.h"
#include "TLorentzVector.h"

//C++
#include <memory>
#include <bitset>

class TriggerValidationAnaProcessor : public Processor {

public:
	TriggerValidationAnaProcessor(const std::string& name, Process& process);
    ~TriggerValidationAnaProcessor();
    virtual bool process(IEvent* ievent);

    virtual void initialize(TTree* tree);

    virtual void finalize();

    virtual void configure(const ParameterSet& parameters);

    enum singleTriggerTypes {single2, single3};

    enum hodoLayers {botLayer2, botLayer1, topLayer1, topLayer2};
    enum hodoXIndice {HODO_LX_1, HODO_LX_CL_12, HODO_LX_2, HODO_LX_CL_23, HODO_LX_3, HODO_LX_CL_34, HODO_LX_4, HODO_LX_CL_45, HODO_LX_5};

    typedef struct{
    	std::bitset<9> patternSet;
    } hodoPattern;

	std::pair<int, int> xHolePairs[8] = { std::pair<int, int>(0, 0), std::pair<
			int, int>(1, -1), std::pair<int, int>(1, 1), std::pair<int, int>(2,
			-1), std::pair<int, int>(2, 1), std::pair<int, int>(3, -1),
			std::pair<int, int>(3, 1), std::pair<int, int>(4, 0)};

	double getMaxEnergyAtHodoHole(std::vector<double> vectEnergy);

    std::map<int, hodoPattern> buildHodoPatternMap(std::vector<HodoHit*> hodoHitVect);

    typedef struct{
    	std::bitset<9> singleTriggerSet;
    } singleTriggerTags;

    singleTriggerTags buildSingle2TriggerTags(CalCluster* ecalCluster);
    singleTriggerTags buildSingle3TriggerTags(CalCluster* ecalCluster, std::map<int, hodoPattern> hodoPatternMap);
    singleTriggerTags buildSingle3TriggerTags(VTPData::hpsCluster vtpCluster, std::map<int, hodoPattern> hodoPatternMap);


    typedef struct{
    	std::bitset<3> feeTriggerSet;
    } feeTriggerTags;

    feeTriggerTags buildFeeTriggerTags(CalCluster* ecalCluster);

    double calculatePDE(int ix, singleTriggerTypes type);

    bool geometryHodoL1L2Matching(hodoPattern layer1, hodoPattern layer2);

    bool geometryEcalHodoMatching(int x, hodoPattern layer1, hodoPattern layer2);

private:
    //Containers to hold histogrammer info
    TriggerValidationAnaHistos* histos{nullptr};
    std::string  histCfgFilename_;

    //TODO Change this to be held from HPSEvent
    TTree* tree_{nullptr};
    TBranch* bvtpData_{nullptr};
    TBranch* btsData_{nullptr};
    TBranch* becalClusters_{nullptr};
    TBranch* bhodoHits_{nullptr};
    TBranch* bhodoClusters_{nullptr};
    TBranch* btrks_{nullptr};

    VTPData* vtpData_{};
    TSData* tsData_{};
    std::vector<CalCluster*> * ecalClusters_{};
    std::vector<HodoHit*> * hodoHits_{};
    std::vector<HodoCluster*> * hodoClusters_{};
    std::vector<Track*> * trks_{};

    std::string anaName_{"triggerValidationAna"};
    std::string vtpColl_{"VTPBank"};
    std::string tsColl_{"TSBank"};
    std::string ecalClusColl_{"EcalClustersCorr"};
    std::string hodoHitColl_{"RecoHodoHits"};
    std::string hodoClusColl_{"RecoHodoClusters"};
    std::string trkColl_{"GBLTracks"};

    //Debug level
    int debug_{0};
    double beamE_{4.55};

    const unsigned int single2Bits = 0x1F;
    const unsigned int single3Bits = 0x1FF;

    const unsigned int feeBits = 0x7;

    //Cut functions for X
    TF1 *func_pos_top_topCutX;
    TF1 *func_pos_top_botCutX;

    TF1 *func_neg_top_topCutX;
    TF1 *func_neg_top_botCutX;

    TF1 *func_pos_bot_topCutX;
    TF1 *func_pos_bot_botCutX;

    TF1 *func_neg_bot_topCutX;
    TF1 *func_neg_bot_botCutX;

    //Cut functions for Y
    TF1 *func_pos_top_topCutY;
    TF1 *func_pos_top_botCutY;

    TF1 *func_neg_top_topCutY;
    TF1 *func_neg_top_botCutY;

    TF1 *func_pos_bot_topCutY;
    TF1 *func_pos_bot_botCutY;

    TF1 *func_neg_bot_topCutY;
    TF1 *func_neg_bot_botCutY;

    //Parameters of cut functions for X
    double pos_top_topCutX[2] = {60.5386, 0.804057};
    double pos_top_botCutX[2] = {-17.872, 0.81275};
    double neg_top_topCutX[2] = {17.4556, 0.832049};
    double neg_top_botCutX[2] = {-61.7584, 0.935637};
    double pos_bot_topCutX[2] = {37.756, 0.991653};
    double pos_bot_botCutX[2] = {-22.4246, 0.942275};
    double neg_bot_topCutX[2] = {30.9072, 0.862994};
    double neg_bot_botCutX[2] = {-32.0523, 0.926463};
    double pos_top_topCutY[2] = {11.8889, 0.950876};
    double pos_top_botCutY[2] = {-11.4137, 0.977692};
    double neg_top_topCutY[2] = {13.1466, 0.908087};
    double neg_top_botCutY[2] = {-11.934, 1.02725};
    double pos_bot_topCutY[2] = {12.1851, 0.99691};
    double pos_bot_botCutY[2] = {-11.4335, 0.959224};
    double neg_bot_topCutY[2] = {12.1802, 1.02709};
    double neg_bot_botCutY[2] = {-10.432, 0.939808};

    // Without track-cluster matching
    int passSingle2Top{0};
    int failSingle2Top{0};
    int failSingle2PassSinge3Top{0};

    int passSingle2Bot{0};
    int failSingle2Bot{0};
    int failSingle2PassSinge3Bot{0};

    int passSingle3Top{0};
    int failSingle3Top{0};
    int passSingle3Bot{0};
    int failSingle3Bot{0};

    int passSinge2Single3Top{0};
    int passSinge2Single3Bot{0};

    int passFee{0};
    int failFee{0};

    // With track-cluster matching
    int passSingle3MatchedClusterTop{0};
    int failSingle3MatchedClusterTop{0};
    int passSingle3MatchedClusterBot{0};
    int failSingle3MatchedClusterBot{0};

    int tsSl2T{0};
    int tsSl3T{0};
    int tsSl2TSl3T{0};
    int tsSl2TSl3F{0};
    int tsSl2FSl3T{0};
    int tsSl2FSl3F{0};

    int tsSl2T_P{0};
    int tsSl3T_P{0};
    int tsSl2TSl3T_P{0};
    int tsSl2TSl3F_P{0};
    int tsSl2FSl3T_P{0};
    int tsSl2FSl3F_P{0};

    int tsSl2T_PN{0};
    int tsSl3T_PN{0};
    int tsSl2TSl3T_PN{0};
    int tsSl2TSl3F_PN{0};
    int tsSl2FSl3T_PN{0};
    int tsSl2FSl3F_PN{0};

    int tsSl2T_1P1N{0};
    int tsSl3T_1P1N{0};
    int tsSl2TSl3T_1P1N{0};
    int tsSl2TSl3F_1P1N{0};
    int tsSl2FSl3T_1P1N{0};
    int tsSl2FSl3F_1P1N{0};

};

#endif
