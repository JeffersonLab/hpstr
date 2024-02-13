#ifndef __T_ANAPROCESSOR_H__
#define __CLUSTERCOMPARE_ANAPROCESSOR_H__

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


class ClusterCompareAnaProcessor : public Processor {

    public:

        ClusterCompareAnaProcessor(const std::string& name, Process& process);

        ~ClusterCompareAnaProcessor();

        /**
         *
         *THIS METHOD FILLS SEVERAL HISTOGRAMS FOR EACH EVENT DIRECTED AT EVALUATING CLUSTERING PERFORMANCE. FOR OUR TRACKING
         *VARIABLES WE PLOT Z0 VS THE NUMBER OF SHARED HITS WITH AND WITHOUT A CLUSTER CUT (WE EXPECT A GOOD RECONSTRUCTION TO HAVE
         *HAVE A LOW NUMBER OF SHARED CLUSTERS, ESPECIALLY AROUND THE Z0 VERTEX AND THAT THIS WOULD FURTHER IMPROVE WITH HIGH P TRACKS)
         *WE ALSO DIRECTLY COMPARE THE REGULAR AND TRANSVERSE MOMENTUM DISTRIBUTIONS FOR THE TWO METHODS.
         *
         *TO FURTHER CHARACTERIZE THE CLUSTERS WE CAPTURE BASIC INFORMATION: CLUSTER AMPLITUDE, TIME, CHARGE, and STRIP CLUSTER POSITION
         *AND DISTANCES. WE DO THIS FOR SHARED AND UNSHARED CLUSTERS. WE LARGELY IGNORE NTD, OR NEXT TO DEAD, ANALYSIS IN THIS PROCESSOR.
         *
         *THIS PROCESSOR IS UNDENIABLY EXTREMELY SIMILAR TO CLUSTERANAPROCESSOR, HAVING BEEN ITS OFFSPRING. THE MAIN DISTINCTION IS THAT
         *THIS PROCESSOR IMPLEMENTS AN IDENTITY TRACKER (ident_) WHICH, ATM, IS IMPLEMENTED TO DETECT WHETHER AN EVENT IS IN TYPE 1 OR TYPE2
         *ALONG WITH A SHORT MACRO WHICH ASSOCIATES TO DIFFERENTLY RECONSTRUCTED ROOT FILES A IDENTIFICATION MONICER, THIS ALLOWS FOR QUICK
         *DIRECT COMPARISON OF TWO MEANS OF RECONSTRUCTION.
         *
         * */

        virtual bool process(IEvent* ievent);

        virtual void initialize(TTree* tree);

        /**
         *
         *THE REMAINING METHODS (UP TO TRACKPLOT) ALL PLOT STANDARD HISTOGRAMS DESCRIBED IN THE DESCRIPTION TO THE PROCESS METHOD.
         *THEY INCLUDE STANDARD FUNCTION CALLS YOU WOULD EXPECT IN A ROOT MACRO AND AFFORD SOME MORE CONTROL THAN A HISTOMANAGER AND
         *REGION SELECTOR COMBO.
         *FEASIBLE FOR A LIMITED NUMBER OF COLLECTION CUTS.
         *
         * */

        virtual void PlotClusterLayers();  

        virtual void PlotClusterCharges();
        
        virtual void PlotClusterTimes();
        
        virtual void PlotClusterPositions();
        
        virtual void TrackMomenta();
        
        virtual void TrackTransverseMomenta(); 

        /**
         *
         *THIS METHOD FILLS THE COLLECTION OF DEAD CHANNEL IDS GIVEN AN INPUT FILENAME. REQUIRED FOR NTD PLOTS, BUT LARGELY DISUSED IN THIS PROCESSOR AS OF YET.
         *
         * */

        virtual void fillDeads();

        /**
         *
         *THIS METHOD GITS THE STRIP COUNT OUT OF ROUGHLY 25000 GIVEN THE FEB AND HYBRID IDS AND THE STRIP NO WRT THESE IDS
         *
         * */

        virtual int GetStrip(int feb, int hyb,int strip);

        /**
         *
         *THE FINALIZE METHOD CALLS ALL THE PLOTTING MACROS ABOVE. THEY ARE CREATED INTO PNGS INTO THE REPOSITORY THE PROCESSOR IS CALLED IN.
         *
         * */

        virtual void finalize();

        virtual void configure(const ParameterSet& parameters);

    private:

        //Containers to hold histogrammer info
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


        //std::string anaName_{"ClusterAna"};
        int debug_{0};
        int isMC_{0};
        float ident_{1.0};
        std::string badchann_{""};
};

#endif
