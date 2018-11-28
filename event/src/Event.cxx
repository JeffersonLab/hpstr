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

    // Add a branch with the given name to the event tree.
    tree_->Branch(name.c_str(), collection, 1000000, 3);  
}
