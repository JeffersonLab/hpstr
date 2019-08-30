#ifndef __IEVENT_H__
#define __IEVENT_H__


#include "TObject.h"

class IEvent {

 public: 
  virtual ~IEvent(){};

  virtual void add(const std::string name, TObject* object) = 0;
  
};


#endif
