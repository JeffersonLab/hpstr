#ifndef __TRIGGERVALIDATION_ANAPROCESSOR_H__
#define __TRIGGERVALIDATION_ANAPROCESSOR_H__

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
    double pos_top_topCutX[2] = {56.2217, 0.967705};
    double pos_top_botCutX[2] = {-12.6351, 0.837295};
    double neg_top_topCutX[2] = {26.4545, 0.821466};
    double neg_top_botCutX[2] = {-38.124, 0.959698};
    double pos_bot_topCutX[2] = {42.585, 1.00094};
    double pos_bot_botCutX[2] = {-36.0758, 0.944868};
    double neg_bot_topCutX[2] = {43.0477, 0.917774};
    double neg_bot_botCutX[2] = {-20.3044, 1.0138};
    double pos_top_topCutY[2] = {14.2595, 0.930762};
    double pos_top_botCutY[2] = {-11.0683, 0.978763};
    double neg_top_topCutY[2] = {14.0088, 0.924618};
    double neg_top_botCutY[2] = {-14.138, 1.05068};
    double pos_bot_topCutY[2] = {9.80912, 0.978079};
    double pos_bot_botCutY[2] = {-14.2543, 0.924373};
    double neg_bot_topCutY[2] = {13.6617, 1.06499};
    double neg_bot_botCutY[2] = {-20.0764, 0.789965};

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

    int passFee{0};
    int failFee{0};

    // With track-cluster matching
    int passSingle3MatchedClusterTop{0};
    int failSingle3MatchedClusterTop{0};
    int passSingle3MatchedClusterBot{0};
    int failSingle3MatchedClusterBot{0};

};

#endif
