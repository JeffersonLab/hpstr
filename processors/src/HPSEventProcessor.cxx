#include "HPSEventProcessor.h"
#include <iostream>

HPSEventProcessor::HPSEventProcessor(const std::string& name, Process& process)
  : Processor(name,process) {
}

HPSEventProcessor::~HPSEventProcessor(){
}

void HPSEventProcessor::initialize(TTree*) {
}

void HPSEventProcessor::process(IEvent* ievent) {
  HpsEvent* event = static_cast<HpsEvent*>(ievent);

  std::cout<<"This is the HPSEventProcessor"<<std::endl;
  
}
  

void HPSEventProcessor::finalize() {
}


DECLARE_PROCESSOR(HPSEventProcessor);
