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
        debug_          = parameters.getInteger("debug");
        trkCollName_    = parameters.getString("trkCollName");
        histCfgFilename_ = parameters.getString("histCfg");
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

}

bool TrackingAnaProcessor::process(IEvent* ievent) {

    // Loop over all the LCIO Tracks and add them to the HPS event.
    for (int itrack = 0; itrack < tracks_->size(); ++itrack) {

        // Get a track
        Track* track = tracks_->at(itrack);

        if(debug_ > 0)
        {
            std::cout<<"========================================="<<std::endl;
            std::cout<<"========================================="<<std::endl;
            std::cout<<"Track params:           "<<std::endl;
            track->Print();
        }

        trkHistos_->Fill1DHistograms(track);
        trkHistos_->Fill2DHistograms(track);
    }//Loop on tracks

    trkHistos_->Fill1DHisto("n_tracks_h",tracks_->size());

    return true;
}

void TrackingAnaProcessor::finalize() { 

    trkHistos_->saveHistos(outF_);
    delete trkHistos_;
    trkHistos_ = nullptr;
    //trkHistos_->Clear();

}

DECLARE_PROCESSOR(TrackingAnaProcessor); 
