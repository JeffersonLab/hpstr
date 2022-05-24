#ifndef __VTX_ANAPROCESSOR_H__
#define __VTX_ANAPROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "TSData.h"
#include "Vertex.h"
#include "HistoManager.h"
#include "VtxAnaHistos.h"

#include "FlatTupleMaker.h"
#include "BaseSelector.h"


//ROOT
#include "Processor.h"
#include "TFile.h"
#include "TTree.h"
#include "TRefArray.h"
#include "TBranch.h"
#include "TVector3.h"
#include "TLorentzVector.h"

//C++
#include <memory>
#include <vector>

class VtxAnaProcessor : public Processor {

public:
	VtxAnaProcessor(const std::string& name, Process& process);
    ~VtxAnaProcessor();
    virtual bool process(IEvent* ievent);

    virtual void initialize(TTree* tree);

    virtual void finalize();

    virtual void configure(const ParameterSet& parameters);

private:
    std::shared_ptr<AnaHelpers> _ah;

    std::shared_ptr<BaseSelector> vtxSelector;
    std::string selectionCfg_;

    //Containers to hold histogrammer info
    VtxAnaHistos* histos{nullptr};
    std::string  histCfgFilename_;

    //TODO Change this to be held from HPSEvent
    TTree* tree_{nullptr};
    TBranch* btsData_{nullptr};
    TBranch* btrks_{nullptr};
    TBranch* bvtxs_{nullptr};
    TBranch* btcvtxs_{nullptr};

    TSData* tsData_{};
    std::vector<Track*>  * trks_{};
    std::vector<Vertex*> * vtxs_{};
    std::vector<Vertex*> * tcvtxs_{};

    std::string anaName_{"vtxAna"};
    std::string tsColl_{"TSBank"};
    std::string trkColl_{"KalmanFullTracks"};
    std::string vtxColl_{"Vertices"};
    std::string tcvtxColl_{"TCVertices"};

    //Debug Level
    int debug_{0};
    double timeOffset_{-999};
    double beamE_{4.55};
    int isData_{1};

    // save variables for vertices
    std::shared_ptr<FlatTupleMaker> treeVertex;
};

#endif
