/**
 * @author Tongtong Cao, UNH
 */
#include "RecoTrackAnaProcessor.h"
#include <iostream>

RecoTrackAnaProcessor::RecoTrackAnaProcessor(const std::string& name, Process& process) : Processor(name,process){}
//TODO CHECK THIS DESTRUCTOR
RecoTrackAnaProcessor::~RecoTrackAnaProcessor(){}


void RecoTrackAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring RecoTrackAnaProcessor" << std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        trkColl_    = parameters.getString("trkColl");
        //trackHitColl_     = parameters.getString("trackHitColl");
        histCfgFilename_ = parameters.getString("histCfg");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void RecoTrackAnaProcessor::initialize(TTree* tree) {
    tree_= tree;
    // init histos
    histos = new RecoTrackAnaHistos(anaName_.c_str());
    histos->loadHistoConfig(histCfgFilename_);
    histos->doTrackComparisonPlots(false);
    histos->DefineHistos();

    // init TTree
    tree_->SetBranchAddress(trkColl_.c_str() , &tracks_, &btracks_);
    tree_->SetBranchAddress(vtxColl_.c_str()  , &vtxs_    , &bvtxs_    );

}

bool RecoTrackAnaProcessor::process(IEvent* ievent) {

    double weight = 1.;

    int n_tracks = tracks_->size();
    histos->Fill1DHisto("n_tracks_h",n_tracks);

    for (int itrack = 0; itrack < n_tracks; ++itrack) {
        // Get a track
        Track* track = tracks_->at(itrack);
        //int n2dhits_onTrack = !track->isKalmanTrack() ? track->getTrackerHitCount() * 2 : track->getTrackerHitCount();

        Track* truth_track = nullptr;

        if(debug_ > 0)
        {
            std::cout<<"========================================="<<std::endl;
            std::cout<<"========================================="<<std::endl;
            std::cout<<"Track params:           "<<std::endl;
            track->Print();
        }

        histos->Fill1DHistograms(track);
        histos->Fill2DTrack(track);

    }//Loop on tracks



    int n_vtxs = vtxs_->size();
    histos->Fill1DHisto("n_vertices_h",n_vtxs);

    for (int iVertex = 0; iVertex < n_vtxs; iVertex ++){
    	Vertex* vertex = vtxs_->at(iVertex);
        histos->Fill1DVertex(vertex);
        histos->Fill2DHistograms(vertex);
    }


    return true;
}

void RecoTrackAnaProcessor::finalize() {

    histos->saveHistos(outF_, anaName_.c_str());
    delete histos;
    histos = nullptr;
}

DECLARE_PROCESSOR(RecoTrackAnaProcessor);
