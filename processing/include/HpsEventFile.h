#ifndef __HPSEVENT_FILE_H
#define __HPSEVENT_FILE_H

#include "IEventFile.h"
#include "HpsEvent.h"
#include "TTreeReader.h"
#include "TFile.h"
#include "TTree.h"


class HpsEventFile : public IEventFile {

 public:

  virtual ~HpsEventFile();
  HpsEventFile(const std::string ifilename, const std::string& ofilename);
  virtual bool nextEvent();
  void setupEvent(IEvent* ievent);
  void resetOutputFileDir() { ofile_->cd();}
  TFile* getOutputFile() { return ofile_;}
  void close();


 private:

  HpsEvent* event_{nullptr};
  int entry_{0};
  int maxEntries_{0};
  TFile* ofile_{nullptr};
  TFile* rootfile_{nullptr};
  TTree* intree_{nullptr};
  
  
  //TTreeReader* ttree_reader;
  
};


#endif
