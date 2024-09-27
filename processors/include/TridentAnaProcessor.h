#ifndef __TRIDENT_ANAPROCESSOR_H__
#define __TRIDENT_ANAPROCESSOR_H__


//HPSTR
#include "HpsEvent.h"
#include "Collections.h"
#include "EventHeader.h"
#include "Vertex.h"
#include "Track.h"
#include "TrackerHit.h"
#include "Particle.h"
#include "Processor.h"
#include "BaseSelector.h"
#include "TrackHistos.h"
#include "FlatTupleMaker.h"
#include "AnaHelpers.h"


//ROOT
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TVector3.h"

//C++ 
#include <memory>

class TridentAnaProcessor : public Processor {
    
public:
    TridentAnaProcessor(const std::string& name, Process& process);
    ~TridentAnaProcessor();
    virtual bool process(IEvent* ievent);

    virtual void initialize(TTree* tree);
    
    virtual void finalize();
    
    virtual void configure(const ParameterSet& parameters);

private:
    
    std::shared_ptr<BaseSelector> vtxSelector;
    std::shared_ptr<BaseSelector> trkSelector;
    std::vector<std::string> regionSelections_;
    
    std::string selectionCfg_;
    std::string trkSelCfg_;


    TBranch* bfspart_{nullptr};
    TBranch* bvtxs_{nullptr};
    TBranch* btrks_{nullptr};
    TBranch* bevth_{nullptr};
    
    std::vector<Vertex*> * vtxs_{};
    std::vector<Particle*> * fspart_{};
    std::vector<Track*>  * trks_{};
    EventHeader* evth_{nullptr};
    
    std::string anaName_{"vtxAna"};
    std::string vtxColl_{"Vertices"};
    std::string fspartColl_{"FinalStateParticles"};
    std::string trkColl_{"GBLTracks"};
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
