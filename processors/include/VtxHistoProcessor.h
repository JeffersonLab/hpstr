#ifndef __VTX_HISTOPROCESSOR_H__
#define __VTX_HISTOPROCESSOR_H__

//HPSTR
#include "Processor.h"
#include "HistogramHelpers.h"

//ROOT
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TAxis.h"

//CPLUSPLUS
#include <map>
#include <vector>
#include <memory>


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

//Rebin factor
int rebin_{1};

//Selection folder

std::string selection_{""};

//2D histos to project

std::vector<std::string> projections_;

//Map storing the 2D histograms

std::map<std::string,TH2F*> _histos2d;
typedef std::map<std::string,TH2F*>::iterator it2d_;

//Map storing the 1D biases and resolutions

std::map<std::string,TH1F* > _histos1d;
typedef std::map<std::string,TH1F*>::iterator it1d_;

};


#endif
