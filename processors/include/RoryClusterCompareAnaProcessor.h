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

        virtual bool process(IEvent* ievent);

        virtual void initialize(TTree* tree);
        
        virtual void Plot1();
        
        virtual void Plot2();
        
        virtual void Plot3();
        
        virtual void Plot4();

        //virtual void Plot5();

        //virtual void Plot6();

        //virtual void Plot7();

        virtual void TrackPlot1();
        
        virtual void TrackPlot2();
        
        virtual void fillDeads();

        virtual int GetStrip(int feb, int hyb,int strip);

        virtual void finalize();

        virtual void configure(const ParameterSet& parameters);

    private:

        //Containers to hold histogrammer info
        //RawSvtHitHistos* histos{nullptr};
        ModuleMapper * mmapper_;
       
        TTree* tree_;
        TBranch* bClusters_{nullptr};
        TBranch* bClustersKF_{nullptr};
        TBranch* bsvtraw_{nullptr};
        TBranch* btracks_{nullptr};
        TBranch* bident_{nullptr};

        int layer_{-1};
        int module_{-1};

        //FOR THE FIRST FILE

        TH1F* layers1_;
        TH1F* layersOnTrk1_;
        TH1F* layersOffTrk1_;
        TH1F* charges1_;
        TH1F* chargesOnTrk1_;
        TH1F* chargesOffTrk1_;

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

        float pcut_{-1.0};
 
        bool doingTracks_{false};

        float Deads_[24576];

        std::vector<TrackerHit*> * Clusters_{};
        std::vector<TrackerHit*> * ClustersKF_{};
        std::vector<RawSvtHit*> * svtraw_{};
        std::vector<Track*> * tracks_{};


        //std::string anaName_{"roryClusterAna"};
        int debug_{0};
        int isMC_{0};
        float ident_{1.0};
};

#endif
