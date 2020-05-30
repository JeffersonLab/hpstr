#include "FindableTrackProcessor.h" 

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <iostream> 

/*~~~~~~~~~~~*/
/*   hpstr   */
/*~~~~~~~~~~~*/
#include "Event.h"
#include "FindableTrack.h" 
#include "MCParticle.h" 
#include "MCTrackerHit.h" 

/*~~~~~~~~~~*/
/*   ROOT   */
/*~~~~~~~~~~*/
#include "TTree.h" 

FindableTrackProcessor::FindableTrackProcessor(const std::string& name, Process& process) 
    : Processor(name, process) { 
}

FindableTrackProcessor::~FindableTrackProcessor() { 
    clear(); 
    delete findable_tracks_;  
}

void FindableTrackProcessor::configure(const ParameterSet& parameters) {

    try {
        mc_particle_col_ = parameters.getString("mc_particle_col");
        sim_tracker_hit_col_ = parameters.getString("sim_tracker_hits_col");  
    } catch (std::runtime_error& error) {
        std::cerr << "Error: " << error.what() << std::endl;
    }

}

void FindableTrackProcessor::initialize(TTree* tree) {
    
    auto b_mc_particles{tree->GetBranch(mc_particle_col_.c_str())};
    b_mc_particles->SetAddress(&mc_particles_); 

    auto b_sim_tracker_hits{tree->GetBranch(sim_tracker_hit_col_.c_str())}; 
    b_sim_tracker_hits->SetAddress(&sim_tracker_hits_);

    // NOTE: This is a hack that allows me to write a ROOT collection 
    // when processing a ROOT file. This will be removed once the 
    // processing class is made more generic. 
    output_tree_ = tree->CloneTree();  
    output_tree_->Branch("FindableTracks", &findable_tracks_); 

}

bool FindableTrackProcessor::process(IEvent* event) {

    // Clear the hit map to remove any previous relations 
    hitMap_.clear();

    // Clear any previously created tracks
    clear();  

    // If the collection of sim tracker hits doesn't exists, skip the event.
    if (sim_tracker_hits_->empty()) return false; 

    createHitMap(sim_tracker_hits_); 
    for (const auto& mc_particle : *mc_particles_) {

        // If the MC particle is neutral, skip it. 
        if ( abs(mc_particle.second->getCharge()) != 1) continue; 
    
        // Check if the track is findable
        if (hitMap_.count(mc_particle.first) == 1) {

        
            // Check if the track is findable
            isFindable(mc_particle.first, hitMap_[mc_particle.first]); 

        }
    }
    output_tree_->Fill(); 
}

void FindableTrackProcessor::finalize() {
    output_tree_->AutoSave(); 
}

void FindableTrackProcessor::createHitMap(const std::vector< MCTrackerHit* >* hits) { 
    
    // Loop over all sim tracker hits and check which layers, if any, the sim
    // particle deposited energy in.
    std::vector< int > hit_count(14, 0); 
    for ( const auto& hit : *hits ) {

        // Get the LCIO ID associated with a specific sim particle
        int lcio_id = hit->getLcioID();

        if (hitMap_.count(lcio_id) == 0) {
            // TODO: Fix hardcoded map size. 
            hitMap_[lcio_id] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        } 
       
        hitMap_[lcio_id][hit->getLayer() - 1]++;  
    }
}

void FindableTrackProcessor::isFindable(int lcio_id, std::vector< int > hit_count) {

    int stereo_hit_count{0}; 
    for (int ilayer{0}; ilayer < hit_count.size(); ilayer += 2) { 
        if (hit_count[ilayer]*hit_count[ilayer + 1] != 0) { 
            ++stereo_hit_count;  
        }
    }
    
    bool is_findable = (stereo_hit_count >= 5) ? true : false;
    findable_tracks_->push_back(new FindableTrack(lcio_id, stereo_hit_count, is_findable));  
}


void FindableTrackProcessor::clear() {
   
    for ( auto element : *findable_tracks_ ) { 
        delete element; 
    }
    findable_tracks_->clear(); 

}

DECLARE_PROCESSOR(FindableTrackProcessor) 
