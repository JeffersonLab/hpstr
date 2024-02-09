#ifndef __CLUSTER_ANAPROCESSOR_H__
#define __CLUSTER_ANAPROCESSOR_H__

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


class ClusterAnaProcessor : public Processor {

    public:

        ClusterAnaProcessor(const std::string& name, Process& process);

        ~ClusterAnaProcessor();

        virtual bool process(IEvent* ievent);

        virtual void initialize(TTree* tree);
        
        virtual void Plot1();
        
        virtual void Plot2();
        
        virtual void Plot3();
        
        virtual void Plot4();

        virtual void Plot5();

        virtual void Plot6();

        virtual void Plot7();

        virtual void TrackPlot1();

        //virtual void ClusterFit();

        virtual void fillDeads();

        virtual int GetStrip(int feb, int hyb,int strip);

        //virtual void sample(RawSvtHit* thisHit, std::string word, IEvent* ievent, long t,int i);
        
        //virtual int hitEff(IEvent* ievent, int L);

        //virtual TF1* fourPoleFitFunction(std::string word, int caser);

        //virtual float str_to_float(std::string word);

        //float reverseEngineerTime(float ti, long t);

        //virtual int maximum(int arr[]);
        
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

        int layer_{-1};
        int module_{-1};

        TH1F* layers_;
        TH1F* layersOnTrk_;
        TH1F* layersOffTrk_;
        TH1F* charges_;
        TH1F* chargesOnTrk_;
        TH1F* chargesOffTrk_;

        TH1F* layersNTD_;
        TH1F* layersOnTrkNTD_;
        TH1F* layersOffTrkNTD_;
        TH1F* chargesNTD_;
        TH1F* chargesOnTrkNTD_;
        TH1F* chargesOffTrkNTD_;

        TH1F* positions_;
        TH1F* positionsOnTrk_;
        TH1F* ClusDistances_;
        TH1F* ClusDistancesNTD_;

        TH1F* times_;
        TH1F* timesOnTrk_;
        TH1F* timesOffTrk_;
        TH1F* timesNTD_;
        TH1F* timesOnTrkNTD_;
        TH1F* timesOffTrkNTD_;

        //TRACKING RELATED VARIABLES

        TH2F* Z0VNShare2Hist_;
        TH2F* Z0VNShare2HistCut_;
        TH1F* SharedAmplitudes_;
        TH1F* UnSharedAmplitudes_;
        TH1F* SharedTimes_;
        TH1F* UnSharedTimes_;

        float pcut_{-1.0};
 
        bool doingTracks_{false};

        float Deads_[24576];

        std::vector<TrackerHit*> * Clusters_{};
        std::vector<TrackerHit*> * ClustersKF_{};
        std::vector<RawSvtHit*> * svtraw_{};
        std::vector<Track*> * tracks_{};


        //std::string anaName_{"ClusterAna"};
        int debug_{0};
        int isMC_{0};
};

#endif
