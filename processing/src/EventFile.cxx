/**
 * @file EventFile.cxx
 * @brief Class for managing IO files.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "EventFile.h"

EventFile::EventFile(const std::string ifilename, const std::string ofilename) { 

    // Open the input LCIO file. If the input file can't be opened, throw an 
    // exception. 
    lc_reader_->open(ifilename); 

    // Open the output ROOT file
    ofile_ = new TFile(ofilename.c_str(), "recreate");
}

EventFile::~EventFile() {}

bool EventFile::nextEvent() { 

    // Close out the previous event before moving on.
    if (entry_ > 0) { 
        event_->getTree()->Fill(); 
    }
   
    // Read the next event.  If it doesn't exist, stop processing events.
    if ((lc_event_ = lc_reader_->readNextEvent())  == 0) return false;
    
    event_->setLCEvent(lc_event_); 

    ++entry_; 
    return true; 
}

void EventFile::setupEvent(Event* event) { 
    event_ = event;
    entry_ = 0;  
}

void EventFile::close() { 
    
    // Close the LCIO file that was being processed
    lc_reader_->close();

    // Write the ROOT tree to disk
    event_->getTree()->Write();  
    
    // Close the ROOT file
    ofile_->Close(); 
}
