#ifndef __VERTEX_ANAPROCESSOR_H__
#define __VERTEX_ANAPROCESSOR_H__


//HPSTR
#include "HpsEvent.h"
#include "Collections.h"
#include "EventHeader.h"
#include "Vertex.h"
#include "Track.h"
#include "Particle.h"
#include "Processor.h"
#include "BaseSelector.h"
//#include "TrackHistos.h"
#include "FlatTupleMaker.h"
#include "AnaHelpers.h"


//ROOT
#include "TFile.h"
#include "TTree.h"
#include "TRefArray.h"
#include "TBranch.h"
#include "TVector3.h"
#include "TLorentzVector.h"

//C++
#include <memory>

class MCTupleForClassificationProcessor : public Processor {

public:
	MCTupleForClassificationProcessor(const std::string& name, Process& process);
    ~MCTupleForClassificationProcessor();
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

    std::string anaName_{"mcTupleForClassification"};
    std::string vtxColl_{"TargetConstrainedV0Vertices_KF"};
    std::string trkColl_{"KalmanFullTracks"};
    TTree* tree_{nullptr};

//std::shared_ptr<TrackHistos> _vtx_histos;

//    typedef std::map<std::string,std::shared_ptr<TrackHistos> >::iterator reg_it;

//    std::string histoCfg_{""};

    double beamE_{4.55};
    int isData_{0};
    std::shared_ptr<AnaHelpers> _ah;

    TTree *treeOutput_{nullptr};
    double ele_mom_mag_{-1000.};
    double ele_mom_theta_{-1000.};
    double ele_mom_phi_{-1000.};
    double ele_d0_{-1000.};
    double ele_phi0_{-1000.};
    double ele_omega_{-1000.};
    double ele_tanLambd_{-1000.};
    double ele_chi2_{-1000.};


    double pos_mom_mag_{-1000.};
    double pos_mom_theta_{-1000.};
    double pos_mom_phi_{-1000.};
    double pos_d0_{-1000.};
    double pos_phi0_{-1000.};
    double pos_omega_{-1000.};
    double pos_tanLambd_{-1000.};
    double pos_chi2_{-1000.};

	double vtx_mom_mag_{-1000};
	double vtx_mom_theta_{-1000};
	double vtx_mom_phi_{-1000};
	double vtx_x_{-1000};
	double vtx_y_{-1000};
	double vtx_z_{-1000};
	double vtx_chi2_{-1000};

    double invariant_mass_{-1000.};

    double openingAngle_{-1000.};

    //Debug level
    int debug_{0};
};

#endif
