#ifndef __TRIGGERVALIDATION_ANAPROCESSOR_H__
#define __TRIGGERVALIDATION_ANAPROCESSOR_H__

//#define __WITHSVT__

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
    double pos_top_topCutX[2] = {55.043, 0.954781};
    double pos_top_botCutX[2] = {-26.8894, 0.905789};
    double neg_top_topCutX[2] = {20.0414, 0.775631};
    double neg_top_botCutX[2] = {-41.2197, 0.928236};
    double pos_bot_topCutX[2] = {43.5722, 0.992549};
    double pos_bot_botCutX[2] = {-36.4948, 0.952063};
    double neg_bot_topCutX[2] = {42.37, 0.914492};
    double neg_bot_botCutX[2] = {-19.5229, 1.01893};
    double pos_top_topCutY[2] = {12.0834, 0.934938};
    double pos_top_botCutY[2] = {-7.89792, 0.950394};
    double neg_top_topCutY[2] = {10.9327, 0.947849};
    double neg_top_botCutY[2] = {-11.051, 1.02282};
    double pos_bot_topCutY[2] = {8.33544, 0.964538};
    double pos_bot_botCutY[2] = {-12.8071, 0.943018};
    double neg_bot_topCutY[2] = {11.4682, 1.03756};
    double neg_bot_botCutY[2] = {-16.6485, 0.836709};

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

};

#endif
