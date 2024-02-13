#ifndef __SVTCLUSTER_ANAPROCESSOR_H__
#define __SVTCLUSTER_ANAPROCESSOR_H__

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


class SvtClusterAnaProcessor : public Processor {

    public:

        SvtClusterAnaProcessor(const std::string& name, Process& process);

        ~SvtClusterAnaProcessor();

        /**
         *
         *THIS METHOD FILLS SEVERAL HISTOGRAMS FOR EACH EVENT DIRECTED AT EVALUATING CLUSTERING PERFORMANCE. FOR OUR TRACKING
         *VARIABLES WE PLOT Z0 VS THE NUMBER OF SHARED HITS WITH AND WITHOUT A CLUSTER CUT (WE EXPECT A GOOD RECONSTRUCTION TO HAVE
         *HAVE A LOW NUMBER OF SHARED CLUSTERS, ESPECIALLY AROUND THE Z0 VERTEX AND THAT THIS WOULD FURTHER IMPROVE WITH HIGH P TRACKS)
         *
         *
         *TO FURTHER CHARACTERIZE THE CLUSTERS WE CAPTURE BASIC INFORMATION: CLUSTER AMPLITUDE, TIME, CHARGE, and STRIP CLUSTER POSITION
         *AND DISTANCES. WE DO THIS FOR SHARED AND UNSHARED CLUSTERS. WE PERFORM THIS FOR REGULAR OR NTD (NEXT TO DEAD) CLUSTERS.
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

        virtual void PlotClusterLayersNTD();

        virtual void PlotClusterCharges();

        virtual void PlotClusterChargesNTD();

        virtual void PlotClusterPositions();

        virtual void PlotClusterTimes();

        virtual void PlotClusterTimesNTD();

        virtual void TrackPlot1();

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
        std::string badchann_{""};
};

#endif
