#ifndef __RORYCLUSTERCOMPARE_ANAPROCESSOR_H__
#define __RORYCLUSTERCOMPARE_ANAPROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "RawSvtHit.h"
#include "TrackerHit.h"
#include "RawSvtHitHistos.h"
#include "AnaHelpers.h"
#include "Event.h"
#include "BaseSelector.h"
#include "RawSvtHitHistos.h"
#include "EventHeader.h"
#include "VTPData.h"
#include "TSData.h"
#include "CalCluster.h"
#include "Track.h"
#include "TrackerHit.h"
#include "Collections.h"
#include "HpsEventFile.h"
#include "HpsEvent.h"

//#include <IMPL/TrackerHitImpl.h>"
//ROOT

#include "Processor.h"
#include "TClonesArray.h"
#include "TBranch.h"
#include "TTree.h"
#include "TFile.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TAxis.h"
#include "TROOT.h"
#include "TPad.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TProfile.h"

class TTree;


class RoryClusterCompareAnaProcessor : public Processor {

    public:

        RoryClusterCompareAnaProcessor(const std::string& name, Process& process);

        ~RoryClusterCompareAnaProcessor();

        virtual bool process();

	virtual bool process(IEvent* event) {return true;};

        virtual void initialize(std::string inFilename, std::string outFilename);
       	
	virtual void initialize(TTree* tree) {};

        virtual void Plot1();
        
        virtual void Plot2();
        
        virtual void Plot3();
        
        virtual void Plot4();

	virtual void loop(IEvent* ievent1,IEvent* ievent2);

	virtual void WriteRoot();

	//virtual void Plot5();

        //virtual void Plot6();

        //virtual void Plot7();

        virtual void TrackPlot1();
        
        virtual void TrackPlot2();
    
	virtual void PlotShared1();

        virtual void fillDeads();

        virtual int GetStrip(int feb, int hyb,int strip);

        virtual void finalize();

        virtual void configure(const ParameterSet& parameters);

    private:

    	HpsEventFile* file1_{nullptr};
	HpsEventFile* file2_{nullptr};
	HpsEvent event1_;
	HpsEvent event2_;
	TTree* tree1_;
	TTree* tree2_;

	//Containers to hold histogrammer info
        //RawSvtHitHistos* histos{nullptr};
        ModuleMapper * mmapper_;
       
        TBranch* bClusters1_{nullptr};
        TBranch* bClustersKF1_{nullptr};
        TBranch* bsvtraw1_{nullptr};
        TBranch* btracks1_{nullptr};

	TBranch* bClusters2_{nullptr};
        TBranch* bClustersKF2_{nullptr};
        TBranch* bsvtraw2_{nullptr};
        TBranch* btracks2_{nullptr};


        int layer_{-1};
        int module_{-1};

        //FOR THE FIRST FILE

        TH1F* layers1_;
        TH1F* layersOnTrk1_;
        TH1F* layersOffTrk1_;
        TH1F* charges1_;
        TH1F* chargesOnTrk1_;
        TH1F* chargesOffTrk1_;
	TH2F* charges2D_;

        TH1F* layersNTD1_;
        TH1F* layersOnTrkNTD1_;
        TH1F* layersOffTrkNTD1_;
        TH1F* chargesNTD1_;
        TH1F* chargesOnTrkNTD1_;
        TH1F* chargesOffTrkNTD1_;

        TH1F* positions1_;
        TH1F* positionsOnTrk1_;
        TH1F* ClusDistances1_;
        TH1F* ClusDistancesNTD1_;

        TH1F* times1_;
        TH1F* timesOnTrk1_;
        TH1F* timesOffTrk1_;
        TH1F* timesNTD1_;
        TH1F* timesOnTrkNTD1_;
        TH1F* timesOffTrkNTD1_;

        //FOR THE SECOND FILE
        
        TH1F* layers2_;
        TH1F* layersOnTrk2_;
        TH1F* layersOffTrk2_;
        TH1F* charges2_;
        TH1F* chargesOnTrk2_;
        TH1F* chargesOffTrk2_;

        TH1F* layersNTD2_;
        TH1F* layersOnTrkNTD2_;
        TH1F* layersOffTrkNTD2_;
        TH1F* chargesNTD2_;
        TH1F* chargesOnTrkNTD2_;
        TH1F* chargesOffTrkNTD2_;

        TH1F* positions2_;
        TH1F* positionsOnTrk2_;
        TH1F* ClusDistances2_;
        TH1F* ClusDistancesNTD2_;

        TH1F* times2_;
        TH1F* timesOnTrk2_;
        TH1F* timesOffTrk2_;
        TH1F* timesNTD2_;
        TH1F* timesOnTrkNTD2_;
        TH1F* timesOffTrkNTD2_;

	//SHARED DISTRIBUTIONS
	
	TH1F* DifferenceInPositions_;
	TH1F* DifferenceInCharges_;
	TH2F* DifferenceInPositionsVStripNo_;
	TH2F* DifferenceInPositionVsCharge_;
        TH2F* DifferenceInPositionVsChargeNTD_;
	TH2F* MinComboPosition0_;
	TH2F* MinComboPosition1_;
	TH2F* MinComboPosition2_;
	TH2F* MinComboChargeNTD_;
	TH2F* DeltaTVsDeltaCharge_;
	TH2F* DeltaTVsDeltaChargeV2_;
        TH2F* ChargeShared2D_;
	//TRACKING RELATED VARIABLES

        TH2F* Z0VNShare2Hist1_;
        TH2F* Z0VNShare2HistCut1_;
        TH1F* SharedAmplitudes1_;
        TH1F* UnSharedAmplitudes1_;
        TH1F* SharedTimes1_;
        TH1F* UnSharedTimes1_;
        TH1F* TrackMomentumInTime1_;
        TH1F* TrackMomentumOutTime1_;
        TH1F* TrackMomentumAllTime1_;
        TH1F* TrackMomentumTInTime1_;
        TH1F* TrackMomentumTOutTime1_;
        TH1F* TrackMomentumTAllTime1_;
	TH1F* HitsOnTrack1_;

        TH2F* Z0VNShare2Hist2_;
        TH2F* Z0VNShare2HistCut2_;
        TH1F* SharedAmplitudes2_;
        TH1F* UnSharedAmplitudes2_;
        TH1F* SharedTimes2_;
        TH1F* UnSharedTimes2_;
        TH1F* TrackMomentumInTime2_;
        TH1F* TrackMomentumOutTime2_;
        TH1F* TrackMomentumAllTime2_;
        TH1F* TrackMomentumTInTime2_;
        TH1F* TrackMomentumTOutTime2_;
        TH1F* TrackMomentumTAllTime2_;
	TH1F* HitsOnTrack2_;

        float pcut_{-1.0};
 
        bool doingTracks_{false};

        float Deads_[24576];

	std::string fileOne_{""};
	
	std::string fileTwo_{""};
        
	std::vector<TrackerHit*> * Clusters1_{};
        std::vector<TrackerHit*> * ClustersKF1_{};
        std::vector<RawSvtHit*> * svtraw1_{};
        std::vector<Track*> * tracks1_{};

	std::vector<TrackerHit*> * Clusters2_{};
        std::vector<TrackerHit*> * ClustersKF2_{};
        std::vector<RawSvtHit*> * svtraw2_{};
        std::vector<Track*> * tracks2_{};

        //std::string anaName_{"roryClusterAna"};
        int debug_{0};
        int isMC_{0};
        float ident_{1.0};
};

#endif
