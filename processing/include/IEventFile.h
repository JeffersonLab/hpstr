#ifndef __IEVENT_FILE_H__
#define __IEVENT_FILE_H__

#include "IEvent.h"

class IEventFile {

 public:
  
  virtual ~IEventFile(){};
  virtual bool nextEvent()=0;
  virtual void setupEvent(IEvent* ievent)=0;
  virtual void close()=0;
  virtual void resetOutputFileDir() = 0;
  
};

#endif
