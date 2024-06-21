#include "HpsEventFile.h"

HpsEventFile::HpsEventFile(const std::string ifilename, const std::string& ofilename){
  rootfile_ = new TFile(ifilename.c_str());
  //ttree_reader = new ("HPS_Event",_rootfile);
  std::cout<<"THE NAME OF THE FILE I PASSED WAS "<<ifilename<<std::endl;
  intree_ = (TTree*)rootfile_->Get("HPS_Event");
  std::cout<<"DID I BREAK HERE IN HPSEVENT1"<<std::endl;
  ofile_    = new TFile(ofilename.c_str(),"recreate");
  std::cout<<"DID I BREAK HERE IN HPSEVENT2"<<std::endl;
  
}

HpsEventFile::~HpsEventFile() {}

void HpsEventFile::setupEvent(IEvent* ievent) {
  event_      = static_cast<HpsEvent*>(ievent);
  //TODO protect the tree pointer
  if (intree_) {
    event_      -> setTree(intree_);
    maxEntries_ = intree_->GetEntriesFast();
  }
  
  entry_      = 0;
}

void HpsEventFile::close() {
  rootfile_->cd();
  rootfile_->Close();
  ofile_->cd();
  ofile_->Close();
  
}

bool HpsEventFile::nextEvent() {
  
  if (entry_ > maxEntries_ - 1)
    return false;

  //TODO Really don't like having the tree associated to the event object. Should be associated to the EventFile.
  intree_->GetEntry(entry_++);
  
  return true;
}
