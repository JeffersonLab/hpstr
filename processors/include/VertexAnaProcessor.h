#ifndef __VERTEX_ANAPROCESSOR_H__
#define __VERTEX_ANAPROCESSOR_H__


//HPSTR
#include "HpsEvent.h"
#include "Collections.h"
#include "EventHeader.h"
#include "Vertex.h"
#include "Track.h"
#include "TrackerHit.h"
#include "MCParticle.h"
#include "Particle.h"
#include "Processor.h"
#include "BaseSelector.h"
#include "TrackHistos.h"
#include "FlatTupleMaker.h"
#include "AnaHelpers.h"


//ROOT
#include "TFile.h"
#include "TTree.h"
#include "TRefArray.h"
#include "TBranch.h"
#include "TVector3.h"

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
    std::vector<std::string> regionSelections_;
    
    std::string selectionCfg_;
    TBranch* bvtxs_{nullptr};
    TBranch* bhits_{nullptr};
    TBranch* btrks_{nullptr};
    TBranch* bmcParts_{nullptr};
    TBranch* bevth_{nullptr};
    
    std::vector<Vertex*> * vtxs_{};
    std::vector<Track*>  * trks_{};
    std::vector<TrackerHit*>  * hits_{};
    std::vector<MCParticle*>  * mcParts_{};
    EventHeader* evth_{nullptr};
    
    std::string anaName_{"vtxAna"};
    std::string vtxColl_{"Vertices"};
    std::string hitColl_{"RotatedHelicalTrackHits"};
    std::string trkColl_{"GBLTracks"};
    std::string mcColl_{"MCParticle"};
    TTree* tree_{nullptr};

    std::shared_ptr<TrackHistos> _vtx_histos;
    
    //Duplicate.. We can make a single class.. ?
    std::map<std::string, std::shared_ptr<BaseSelector> > _reg_vtx_selectors;
    std::map<std::string, std::shared_ptr<TrackHistos> > _reg_vtx_histos;
    std::map<std::string, std::shared_ptr<FlatTupleMaker> > _reg_tuples;
    
    std::vector<std::string> _regions;
    

    typedef std::map<std::string,std::shared_ptr<TrackHistos> >::iterator reg_it;

    std::string histoCfg_{""};
    double timeOffset_{-999};
    //In GeV. Default is 2016 value;
    double beamE_{2.3};
    int isData{0};
    std::shared_ptr<AnaHelpers> _ah;



    //Debug level
    int debug_{0};
};

#endif
