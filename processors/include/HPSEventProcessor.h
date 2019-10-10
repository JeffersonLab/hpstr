#ifndef _HPSEVENT_HEADER_PROCESSOR_H__
#define _HPSEVENT_HEADER_PROCESSOR_H__

#include "TClonesArray.h"

#include "Processor.h"
#include "HpsEvent.h"
#include "TTree.h"



class HPSEventProcessor : public Processor {

 public:

  HPSEventProcessor(const std::string& name, Process& process);
  ~HPSEventProcessor();

  virtual bool process(IEvent* ievent);
  virtual void initialize(TTree* tree);
  virtual void finalize();

 private:
  TClonesArray* header_{nullptr};
  
};

#endif
