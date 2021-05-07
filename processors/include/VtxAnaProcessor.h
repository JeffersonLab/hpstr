#ifndef __VTX_ANAPROCESSOR_H__
#define __VTX_ANAPROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "TSData.h"
#include "Vertex.h"
#include "HistoManager.h"
#include "VtxAnaHistos.h"


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
    //Containers to hold histogrammer info
    VtxAnaHistos* histos{nullptr};
    std::string  histCfgFilename_;

    //TODO Change this to be held from HPSEvent
    TTree* tree_{nullptr};
    TBranch* btsData_{nullptr};
    TBranch* bvtxs_{nullptr};
    TBranch* btcvtxs_{nullptr};

    TSData* tsData_{};
    std::vector<Vertex*> * vtxs_{};
    std::vector<Vertex*> * tcvtxs_{};

    std::string anaName_{"vtxAna"};
    std::string tsColl_{"TSBank"};
    std::string vtxColl_{"Vertices"};
    std::string tcvtxColl_{"TCVertices"};

    //Debug level
    int debug_{0};

    double beamE_{4.55};

    std::vector<Vertex> vect_all_uc_v0s;
    std::vector<Track> vect_all_track_ele;
    std::vector<Track> vect_all_track_pos;

    std::shared_ptr<AnaHelpers> _ah;
};

#endif
