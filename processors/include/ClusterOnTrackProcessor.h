#ifndef __CLUSTERONTRACK_PROCESSOR_H__
#define __CLUSTERONTRACK_PROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "Track.h"


//ROOT
#include "ClusterHistos.h"
#include "Processor.h"
#include "TClonesArray.h"

class TTree;


class ClusterOnTrackProcessor : public Processor {

 public:

  ClusterOnTrackProcessor(const std::string& name, Process& process);

  ~ClusterOnTrackProcessor();

  virtual void process(IEvent* ievent);

  virtual void initialize(TTree* tree);

  virtual void finalize();

 private:

  ClusterHistos* clusterHistos;
  //TODO Change this to be held from HPSEvent
  TTree* tree_;
  TClonesArray* tracks_{nullptr};
  TBranch*      btracks_{nullptr};
  TClonesArray* hits_{nullptr};
  TBranch*      bhits_{nullptr};  
  
};


#endif
