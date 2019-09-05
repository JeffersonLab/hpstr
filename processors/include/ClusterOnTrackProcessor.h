#ifndef __CLUSTERONTRACK_PROCESSOR_H__
#define __CLUSTERONTRACK_PROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "Track.h"
#include "TrackerHit.h"


//ROOT
#include "ClusterHistos.h"
#include "Processor.h"
#include "TClonesArray.h"
#include "TFile.h"

class TTree;


class ClusterOnTrackProcessor : public Processor {

 public:

  ClusterOnTrackProcessor(const std::string& name, Process& process);

  ~ClusterOnTrackProcessor();

  virtual bool process(IEvent* ievent);

  virtual void initialize(TTree* tree);

  virtual void finalize();

 private:

  ClusterHistos* clusterHistos;
  //TODO Change this to be held from HPSEvent
  TTree* tree_;
  std::vector<Track*> *tracks_{};
  TBranch*      btracks_{nullptr};
  std::vector<TrackerHit*> hits_{};
  TBranch*      bhits_{nullptr};
  TFile*        outF_{nullptr};
  
};


#endif
