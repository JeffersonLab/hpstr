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
    branches_[name] = tree_->Branch(name.c_str(), collection, 1000000, 3); 

    // Keep track of which events were added to the event
    collections_[name] = collection;  
}

TClonesArray* Event::getCollection(const std::string name) { 
    
    // Check if the collection already exist
    auto itc = collections_.find(name);
    auto itb = branches_.find(name); 

    if (itc != collections_.end()) { 
        if (itb != branches_.end()) { 
            itb->second->GetEntry(entry_); 
        }
        return itc->second; 
    } else { 
        throw std::runtime_error("Collection not found."); 
    }
}

bool Event::exists(const std::string name) {  
    
    // Search the list of collections to find if it exist. 
    auto it = collections_.find(name); 

    if (it == collections_.end()) return false; 
    else return true; 
}

void Event::Clear() { 
    
    for (auto& collection : collections_) { 
        collection.second->Clear("C"); 
    }
}
