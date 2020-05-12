#include "MCTrackerHitProcessor.h" 

//-----------------//
//   C++  StdLib   //
//-----------------//
#include <iostream>

//----------//
//   LCIO   //
//----------//
#include <EVENT/MCParticle.h>
#include <EVENT/SimTrackerHit.h>
#include <EVENT/LCCollection.h>

//----------//
//   ROOT   //
//----------//
#include "TTree.h"

//-----------//
//   hpstr   //
//-----------//
#include "Event.h"
#include "MCParticle.h" 
#include "MCTrackerHit.h"

MCTrackerHitProcessor::MCTrackerHitProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
}

MCTrackerHitProcessor::~MCTrackerHitProcessor() { 
}

void MCTrackerHitProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring MCTrackerHitProcessor" << std::endl;
    try {

        debug_               = parameters.getInteger("debug", debug_ );
        mc_tracker_hit_col_  = parameters.getString("mc_tracker_hit_col",  mc_tracker_hit_col_);
        sim_tracker_hit_col_ = parameters.getString("sim_tracker_hit_col", sim_tracker_hit_col_);  
    } catch (std::runtime_error& error) {
        std::cout << error.what() << std::endl;
    }
}

void MCTrackerHitProcessor::initialize(TTree* tree) {

    tree->Branch(mc_tracker_hit_col_.c_str(), &sim_tracker_hits_);
}

bool MCTrackerHitProcessor::process(IEvent* ievent) {

    // Cast the event to an LCEvent
    auto event{static_cast<Event*>(ievent)};

    // Get the collection of simulated tracker hits from the LCIO event.
    EVENT::LCCollection* sim_tracker_hits{nullptr};
    try {
        sim_tracker_hits = event->getLCCollection(sim_tracker_hit_col_.c_str());
    } catch (EVENT::DataNotAvailableException e) {
        std::cout << e.what() << std::endl;
    }

    // Clear any previously stored data
    clear(); 

    // Get the simulated tracker hit count
    auto hit_count{sim_tracker_hits->getNumberOfElements()}; 

    for (int ihit{0}; ihit < hit_count; ++ihit) {

        // Get the simulated tracker hits
        auto sim_tracker_hit{static_cast<EVENT::SimTrackerHit*>(sim_tracker_hits->getElementAt(ihit))};

        //Decode the cellid
        // TODO: Move this to a utility class
        auto value{ EVENT::long64( sim_tracker_hit->getCellID0() & 0xffffffff ) | 
                  ( EVENT::long64( sim_tracker_hit->getCellID1() ) << 32 ) };
        decoder_.setValue(value);

        // Add a raw tracker hit to the event
        auto mc_tracker_hit{new MCTrackerHit()};

        // Set sensitive detector identification
        mc_tracker_hit->setLayer(decoder_["layer"]);
        mc_tracker_hit->setModule(decoder_["module"]);

        // Set the position of the hit
        double hitPos[3];
        hitPos[0] = sim_tracker_hit->getPosition()[0];
        hitPos[1] = sim_tracker_hit->getPosition()[1];
        hitPos[2] = sim_tracker_hit->getPosition()[2];
        mc_tracker_hit->setPosition(hitPos);

        // Set the energy deposit of the hit
        mc_tracker_hit->setEdep(sim_tracker_hit->getEDep());

        // Set the pdg of particle generating the hit
        auto mc_particle{sim_tracker_hit->getMCParticle()}; 
        if(mc_particle != nullptr) {    
            mc_tracker_hit->setPDG(sim_tracker_hit->getMCParticle()->getPDG());
            mc_tracker_hit->setLcioID(sim_tracker_hit->getMCParticle()->id()); 
        }

        // Set the time of the hit
        mc_tracker_hit->setTime(sim_tracker_hit->getTime());

        //Push hit onto vector
        sim_tracker_hits_->push_back(mc_tracker_hit);

    }

    return true;
}

void MCTrackerHitProcessor::finalize() { 
}

void MCTrackerHitProcessor::clear() { 
    for (auto hit : *sim_tracker_hits_) delete hit; 
    sim_tracker_hits_->clear();
}

DECLARE_PROCESSOR(MCTrackerHitProcessor); 
