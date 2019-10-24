#ifndef __CLUSTERONTRACK_ANAPROCESSOR_H__
#define __CLUSTERONTRACK_ANAPROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "Track.h"
#include "TrackerHit.h"
#include "Collections.h"

//ROOT
#include "SvtCondHistos.h"
#include "Processor.h"
#include "TClonesArray.h"
#include "TFile.h"

class TTree;


class SvtCondAnaProcessor : public Processor {

 public:

  SvtCondAnaProcessor(const std::string& name, Process& process);

  ~SvtCondAnaProcessor();

  virtual bool process(IEvent* ievent);

  virtual void initialize(TTree* tree);

  virtual void finalize();
  
  virtual void configure(const ParameterSet& parameters);
  
 /* void setBaselineFits(const std::string& baselineFits,const std::string& baselineRun){
    baselineFits_ = baselineFits;
    baselineRun_  = baselineRun;
  };*/
  

 private:
  
  SvtCondHistos* svtCondHistos{nullptr};
  
  std::string baselineFits_{""};
  std::string baselineRun_{""};

  //TODO Change this to be held from HPSEvent
  TClonesArray* rawSvtHits_{nullptr};
 //
  TTree* tree_;
  std::vector<Track*> *tracks_{};
  std::vector<TrackerHit*> hits_{};
  TBranch*      brawSvtHits_{nullptr};
  TFile*        outF_{nullptr};
  
};


#endif
