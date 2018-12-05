/**
 * @file Event.cxx
 * @brief Class defining methods used to access event information and data
 *        collections.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Event.h"

Event::Event() {
    
    // Create the tree
    tree_ = new TTree("HPS_Event", "HPS event tree"); 
}

Event::~Event() {}

void Event::addCollection(const std::string name, TClonesArray* collection) {   

    // Check if the collection has been added
    if (collections_.find(name) != collections_.end()) return; 

    // Add a branch with the given name to the event tree.
    tree_->Branch(name.c_str(), collection, 1000000, 3); 

    // Kepp track of which events were added to the event
    collections_[name] = collection;  
}

void Event::Clear() { 
    
    for (auto& collection : collections_) { 
        collection.second->Clear("C"); 
    }
}
