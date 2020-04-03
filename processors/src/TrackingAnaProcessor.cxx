#include "TrackingAnaProcessor.h"
#include <iomanip>
#include "utilities.h"

TrackingAnaProcessor::TrackingAnaProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
    }

TrackingAnaProcessor::~TrackingAnaProcessor() { 
}

void TrackingAnaProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring TrackingAnaProcessor" << std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        trkCollName_     = parameters.getString("trkCollName");
        histCfgFilename_ = parameters.getString("histCfg");
        doTruth_         = (bool) parameters.getInteger("doTruth");
        truthHistCfgFilename_ = parameters.getString("truthHistCfg");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void TrackingAnaProcessor::initialize(TTree* tree) {

    //Init histos
    trkHistos_ = new TrackHistos(trkCollName_);
    trkHistos_->loadHistoConfig(histCfgFilename_);
    trkHistos_->doTrackComparisonPlots(false);
    trkHistos_->DefineHistos();
    // Init tree
    tree->SetBranchAddress(trkCollName_.c_str(), &tracks_, &btracks_);
    
    
    if (doTruth_) {
        truthHistos_ = new TrackHistos(trkCollName_+"_truthComparison");
        truthHistos_->loadHistoConfig(histCfgFilename_);
        truthHistos_->DefineHistos();
        truthHistos_->loadHistoConfig(truthHistCfgFilename_);
        truthHistos_->DefineHistos();
        truthHistos_->doTrackComparisonPlots(false);
        
        //tree->SetBranchAddress(truthCollName_.c_str(),&truth_tracks_,&btruth_tracks_);
    }

}

bool TrackingAnaProcessor::process(IEvent* ievent) {

    // Loop over all the LCIO Tracks and add them to the HPS event.
    for (int itrack = 0; itrack < tracks_->size(); ++itrack) {

        // Get a track
        Track* track = tracks_->at(itrack);

        //Select Kalman Tracks with at least 10 hits
        if (track->isKalmanTrack()) {
            if (track->getTrackerHitCount() < 10 )
                continue;
        }

        Track* truth_track = nullptr;

        //Get the truth track
        if (doTruth_) { 
            truth_track = (Track*) track->getTruthLink().GetObject();
            if (!truth_track)
                std::cout<<"Warnings::TrackingAnaProcessor::Requested Truth track but couldn't find it in the ntuple"<<std::endl;
        }
        
        if(debug_ > 0)
        {
            std::cout<<"========================================="<<std::endl;
            std::cout<<"========================================="<<std::endl;
            std::cout<<"Track params:           "<<std::endl;
            track->Print();
        }
        
        trkHistos_->Fill1DHistograms(track);
        trkHistos_->Fill2DTrack(track);
        
        if (truthHistos_) {
            truthHistos_->Fill1DHistograms(truth_track);
            truthHistos_->Fill2DTrack(track);
            truthHistos_->Fill1DTrackTruth(track, truth_track);
        }
        
    }//Loop on tracks

    trkHistos_->Fill1DHisto("n_tracks_h",tracks_->size());

    return true;
}

void TrackingAnaProcessor::finalize() { 

    trkHistos_->saveHistos(outF_,trkCollName_);
    delete trkHistos_;
    trkHistos_ = nullptr;

    if (truthHistos_) {
        truthHistos_->saveHistos(outF_,trkCollName_+"_truth");
        delete truthHistos_;
        truthHistos_ = nullptr;
    }
    //trkHistos_->Clear();
}

DECLARE_PROCESSOR(TrackingAnaProcessor); 
