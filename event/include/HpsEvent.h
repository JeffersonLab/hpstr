#ifndef __HPS_EVENT_H_
#define __HPS_EVENT_H_

#include "IEvent.h"
#include "TClonesArray.h"
#include "TTree.h"


class HpsEvent : public IEvent {

 public:
  HpsEvent();
  virtual void add(const std::string name, TObject* object){};
  void addCollection(const std::string name, TClonesArray* collection);
  void setTree(TTree* tree);
  virtual TTree* getTree(){return tree_;}
 
 private:
  TTree* tree_;
};

#endif
