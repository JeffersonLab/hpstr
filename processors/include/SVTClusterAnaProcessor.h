#ifndef __SVTCLUSTER_ANAPROCESSOR_H__
#define __SVTCLUSTER_ANAPROCESSOR_H__

// HPSTR
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

// ROOT
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


class SVTClusterAnaProcessor : public Processor {

    public:

        SVTClusterAnaProcessor(const std::string& name, Process& process);

        ~SVTClusterAnaProcessor();

        /**
         * 
         * This method fills several histograms for each event directed at evaluating clustering performance. For our tracking
         * variables we plot z0 vs the number of shared hits with and without a cluster cut (we expect a good reconstruction to have
         * have a low number of shared clusters, especially around the z0 vertex and that this would further improve with high p tracks)
         * 
         * To further characterize the clusters we capture basic information: cluster amplitude, time, charge, and strip cluster position
         * and distances. We do this for shared and unshared clusters. We perform this for regular or NTD (next to dead) clusters.
         *
         **/
        virtual bool process(IEvent* ievent);

        virtual void initialize(TTree* tree);

        /**
         *
         * The remaining methods (up to TrackPlot) all plot standard histograms described in the description to the process method.
         * They include standard function calls you would expect in a ROOT macro and afford some more control than a HistoManager and
         * RegionSelector combo.
         * Feasible for a limited number of collection cuts.
         *
         **/

        virtual void PlotClusterLayers();

        virtual void PlotClusterLayersNTD();

        virtual void PlotClusterCharges();

        virtual void PlotClusterChargesNTD();

        virtual void PlotClusterPositions();

        virtual void PlotClusterTimes();

        virtual void PlotClusterTimesNTD();

        virtual void TrackPlot();

        /**
         *
         * This method fills the collection of dead channels given an input file name. Required for NTD plots, but largely disused in this processor as of yet.
         *
         **/
        virtual void fillDeads();

        /**
         *
         * This method gets the strip count out of roughly 25000 given the FEB and hybrid IDs and the strip number with respect to these IDs.
         *
         **/
        virtual int GetStrip(int feb, int hyb,int strip);

        /**
         *
         * The finalize method calls all the plotting macros above. They are created into PNGs into the repository the processor is called in.
         * 
         **/
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

        // TRACKING RELATED VARIABLES

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

        int debug_{0};
        int isMC_{0};
        std::string badchann_{""};
};

#endif
