#ifndef __VERTEX_ANAPROCESSOR_H__
#define __VERTEX_ANAPROCESSOR_H__


//HPSTR
#include "HpsEvent.h"
#include "Collections.h"
#include "Vertex.h"
#include "Track.h"
#include "Particle.h"
#include "Processor.h"
#include "BaseSelector.h"
#include "TrackHistos.h"

//ROOT
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"

//C++ 
#include <memory>

class VertexAnaProcessor : public Processor {
    
public:
    VertexAnaProcessor(const std::string& name, Process& process);
    ~VertexAnaProcessor();
    virtual bool process(IEvent* ievent);

    virtual void initialize(TTree* tree);
    
    virtual void finalize();
    
    virtual void configure(const ParameterSet& parameters);

private:
    
    std::shared_ptr<BaseSelector> vtxSelector;
    std::string selectionCfg_;
    TBranch* bvtxs_{nullptr};
    TBranch* btrks_{nullptr};
    
    std::vector<Vertex*> * vtxs_{};
    std::vector<Track*>  * trks_{};
    
    std::string anaName_{"vtxAna"};
    std::string vtxColl_{"Vertices"};
    std::string trkColl_{"GBLTracks"};
    TTree* tree_{nullptr};

    TH1F* nvtxs_;
    
    std::shared_ptr<TrackHistos> _vtx_histos;
    std::string histoCfg_{""};
    double timeOffset_{-999};
    
    //Debug level
    int debug_{0};
};

#endif
