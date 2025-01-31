#include "HpsEventFile.h"

HpsEventFile::HpsEventFile(const std::string ifilename, const std::string& ofilename){
  std::cout << "[HpsEventFile] Opening input ROOT file: " << ifilename << std::endl;
  rootfile_ = new TFile(ifilename.c_str(),"READ");
  intree_ = (TTree*)rootfile_->Get("HPS_Event");
  std::string _ofilename = "output_" + ifilename;
  std::cout << "[HpsEventFile] Creating output ROOT file: " << _ofilename  << std::endl;  
  ofile_    = new TFile(_ofilename.c_str(),"recreate");
  
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

void HpsEventFile::changeInputFile(const std::string ifilename)
{

  if( rootfile_ ){
    rootfile_->cd();
    rootfile_->Close();
  }

  rootfile_ = new TFile(ifilename.c_str(),"READ");  
  intree_ = (TTree*)rootfile_->Get("HPS_Event");
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
