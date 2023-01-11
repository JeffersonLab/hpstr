/**
 * @file EventFile.cxx
 * @brief Class for managing IO files.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */

#include "EventFile.h"
#include "TProcessID.h"

EventFile::EventFile(const std::string ifilename, const std::string& ofilename) { 

    // Open the input LCIO file. If the input file can't be opened, throw an 
    // exception. 
    lc_reader_->open(ifilename); 

    // Open the output ROOT file
    ofile_ = new TFile(ofilename.c_str(), "recreate");
}

EventFile::~EventFile() {}

// Close out the previous event before moving on.
void EventFile::FillEvent() {
    if (entry_ > 0) {
        event_->getTree()->Fill();
    }
    TProcessID::SetObjectCount(objNumRoot_);
}

bool EventFile::nextEvent() { 

    // Read the next event.  If it doesn't exist, stop processing events.
    if ((lc_event_ = lc_reader_->readNextEvent())  == 0) return false;
    
    event_->setLCEvent(lc_event_); 
    event_->setEntry(entry_); 
    ++entry_; 

     objNumRoot_ = TProcessID::GetObjectCount();

    return true; 
}

void EventFile::setupEvent(IEvent* ievent) {
    event_ = static_cast<Event*> (ievent);
    entry_ = 0;  
}

void EventFile::resetOutputFileDir() {
  ofile_->cd();
}

void EventFile::close() { 
    
    // Close the LCIO file that was being processed
    lc_reader_->close();

    // Write the ROOT tree to disk
    ofile_->cd();
    event_->getTree()->Write();  
    
    // Close the ROOT file
    ofile_->Close(); 
}
