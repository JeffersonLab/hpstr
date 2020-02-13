#ifndef __VTX_HISTOPROCESSOR_H__
#define __VTX_HISTOPROCESSOR_H__

//HPSTR
#include "Processor.h"
#include "HistogramHelpers.h"

//ROOT
#include "TFile.h"
#include "TH1.h"


class VtxHistoProcessor : public Processor {

public:

VtxHistoProcessor(const std::string& name, Process& process);

~VtxHistoProcessor();

virtual void configure(const ParameterSet& parameters);

virtual void initialize(std::string inFilename, std::string outFilename);

virtual bool process();

virtual void initialize(TTree* tree) {};

virtual bool process(IEvent* event) { return true;};

virtual void finalize();

private:

TFile* inF_{nullptr};
        
//Debug Level
int debug_{0};

};


#endif
