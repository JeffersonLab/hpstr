/**
 * @file Event.cxx
 * @brief Class defining methods used to access event information and data
 *        collections.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Event.h"

Event::Event() {
    
    // Create the tree
    //tree_ = new TTree("HPS_Event", "HPS event tree");
    
    // Instantiate the event header
    event_header_ = new EventHeader();  
}

Event::~Event() {}

void Event::add(const std::string name, TObject* object) { 

    // Check if the object has been added to the event.
    if (objects_.find(name) != objects_.end()) { 
        object->Copy(*objects_[name]);
        return; 
    }

    // Create a clone of the object
    TObject* cp = object->Clone(); 
    objects_[name] = cp; 

    // Add a branch with the given name to the event tree.
    branches_[name] = tree_->Branch(name.c_str(), cp);
    
    // Copy the object to the event
    object->Copy(*cp); 
}

void Event::addCollection(const std::string name, TClonesArray* collection) {   

    // Check if the collection has been added
    if (objects_.find(name) != objects_.end()) return; 

    // Add a branch with the given name to the event tree.
    branches_[name] = tree_->Branch(name.c_str(), collection, 1000000, 3); 

    // Keep track of which collections were added to the event
    objects_[name] = collection;  
}

TClonesArray* Event::getCollection(const std::string name) { 
    
    // Check if the collection already exist
    auto itc = objects_.find(name);
    auto itb = branches_.find(name); 

    if (itc != objects_.end()) { 
        if (itb != branches_.end()) { 
            itb->second->GetEntry(entry_); 
        }
        return static_cast<TClonesArray*>(itc->second); 
    } else { 
        throw std::runtime_error("Collection not found."); 
    }
}

bool Event::exists(const std::string name) {  
    
    // Search the list of collections to find if it exist. 
    auto it = objects_.find(name); 

    if (it == objects_.end()) return false; 
    else return true; 
}

void Event::Clear() { 
    
    for (auto& collection : objects_) { 
        collection.second->Clear("C"); 
    }
}
