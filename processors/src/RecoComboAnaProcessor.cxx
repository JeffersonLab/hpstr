/**
 * @author Tongtong Cao, UNH
 */
#include "RecoComboAnaProcessor.h"
#include <iostream>

RecoComboAnaProcessor::RecoComboAnaProcessor(const std::string& name, Process& process) : Processor(name,process){}
//TODO CHECK THIS DESTRUCTOR
RecoComboAnaProcessor::~RecoComboAnaProcessor(){}


void RecoComboAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring RecoEcalAnaProcessor" << std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");

        ecalHitColl_     = parameters.getString("ecalHitColl");
        ecalClusColl_    = parameters.getString("ecalClusColl");
        histCfgFilenameEcal_ = parameters.getString("histCfgEcal");

        hodoHitColl_     = parameters.getString("hodoHitColl");
        hodoClusColl_    = parameters.getString("hodoClusColl");
        histCfgFilenameHodo_ = parameters.getString("histCfgHodo");

        trkColl_    = parameters.getString("trkColl");
        vtxColl_     = parameters.getString("vtxColl");
        histCfgFilenameTrack_ = parameters.getString("histCfgTrack");

        fspCollRoot_    = parameters.getString("fspCollRoot");
        histCfgFilenameParticle_ = parameters.getString("histCfgParticle");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void RecoComboAnaProcessor::initialize(TTree* tree) {
    tree_= tree;
    // init histos for Ecal
    histosEcal = new RecoEcalAnaHistos("recoEcalAna");
    histosEcal->loadHistoConfig(histCfgFilenameEcal_);
    histosEcal->DefineHistos();

    // init TTree for Ecal
    tree_->SetBranchAddress(ecalHitColl_.c_str()  , &ecalHits_    , &becalHits_    );
    tree_->SetBranchAddress(ecalClusColl_.c_str() , &ecalClusters_, &becalClusters_);

    // init histos for Hodo
    histosHodo = new RecoHodoAnaHistos("recoHodoAna");
    histosHodo->loadHistoConfig(histCfgFilenameHodo_);
    histosHodo->DefineHistos();

    // init TTree for Hodo
    tree_->SetBranchAddress(hodoHitColl_.c_str()  , &hodoHits_    , &bhodoHits_    );
    tree_->SetBranchAddress(hodoClusColl_.c_str() , &hodoClusters_, &bhodoClusters_);

    // init histos for track and vertex
    histosTrack = new RecoTrackVertexAnaHistos("recoTrackAna");
    histosTrack->loadHistoConfig(histCfgFilenameTrack_);
    histosTrack->doTrackComparisonPlots(false);
    histosTrack->DefineHistos();

    // init TTree for track and vertex
    tree_->SetBranchAddress(trkColl_.c_str() , &tracks_, &btracks_);
    tree_->SetBranchAddress(vtxColl_.c_str()  , &vtxs_    , &bvtxs_    );

    // init histos for final-state particle
    histosParticle = new RecoParticleAnaHistos("recoParticleAna");
    histosParticle->loadHistoConfig(histCfgFilenameParticle_);
    histosParticle->DefineHistos();

    // init TTree for final-state particle
    tree_->SetBranchAddress(fspCollRoot_.c_str() , &fsps_, &bfsps_);

}

bool RecoComboAnaProcessor::process(IEvent* ievent) {

	histosEcal->FillEcalHits(ecalHits_);
	histosEcal->FillEcalClusters(ecalClusters_);

	histosHodo->FillHodoHits(hodoHits_);
	histosHodo->FillHodoClusters(hodoClusters_);

    int n_tracks = tracks_->size();
    histosTrack->Fill1DHisto("n_tracks_h",n_tracks);

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

        histosTrack->Fill1DHistograms(track);
        histosTrack->Fill2DTrack(track);

    }//Loop on tracks

    int n_vtxs = vtxs_->size();
    histosTrack->Fill1DHisto("n_vertices_h",n_vtxs);

    for (int iVertex = 0; iVertex < n_vtxs; iVertex ++){
    	Vertex* vertex = vtxs_->at(iVertex);
    	histosTrack->Fill1DVertex(vertex);
    	histosTrack->Fill2DHistograms(vertex);
    }

    histosParticle->FillFSPs(fsps_);

    return true;
}

void RecoComboAnaProcessor::finalize() {

	histosEcal->saveHistos(outF_, histosEcal->getName());
    delete histosEcal;
    histosEcal = nullptr;

    histosHodo->saveHistos(outF_, histosHodo->getName());
    delete histosHodo;
    histosHodo = nullptr;

    histosTrack->saveHistos(outF_, histosTrack->getName());
    delete histosTrack;
    histosTrack = nullptr;

    histosParticle->saveHistos(outF_, histosParticle->getName());
    delete histosParticle;
    histosParticle = nullptr;
}

DECLARE_PROCESSOR(RecoComboAnaProcessor);
