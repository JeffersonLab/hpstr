#ifndef __TRACK_EFFICIENCYPROCESSOR_H__
#define __TRACK_EFFICIENCYPROCESSOR_H__


//HPSTR
#include "HpsEvent.h"
#include "Collections.h"
#include "EventHeader.h"
#include "Vertex.h"
#include "CalCluster.h"
#include "Track.h"
#include "TrackerHit.h"
#include "Particle.h"
#include "Processor.h"
#include "BaseSelector.h"
#include "TrackEfficHistos.h"
#include "ThreeProngHistos.h"
#include "FlatTupleMaker.h"
//#include "AnaHelpers.h"


//ROOT
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TVector3.h"

//C++ 
#include <memory>

class TrackEfficiencyProcessor : public Processor {
    
public:
    TrackEfficiencyProcessor(const std::string& name, Process& process);
    ~TrackEfficiencyProcessor();
    virtual bool process(IEvent* ievent);

    virtual void initialize(TTree* tree);
    
    virtual void finalize();
    
    virtual void configure(const ParameterSet& parameters);

private:
    
    std::shared_ptr<BaseSelector> cluSelector;
    std::shared_ptr<BaseSelector> trkSelector;
    std::vector<std::string> regionSelections_;
    std::vector<std::string> threeProngSelections_;
    
    std::string selectionCfg_;
    std::string trkSelCfg_;
    std::string cluSelCfg_;


    TBranch* bfspart_{nullptr};
    TBranch* bclus_{nullptr};
    TBranch* btrks_{nullptr};
    TBranch* bevth_{nullptr};
    
    std::vector<CalCluster*> * clus_{};
    std::vector<Particle*> * fspart_{};
    std::vector<Track*>  * trks_{};
    EventHeader* evth_{nullptr};
    
    std::string anaName_{"trkEffAna"};
    std::string cluColl_{"ECalClusters"};
    std::string fspartColl_{"FinalStateParticles"};
    std::string trkColl_{"GBLTracks"};
    TTree* tree_{nullptr};

    std::shared_ptr<TrackEfficHistos> _trkeff_histos;
    
    std::map<std::string, std::shared_ptr<BaseSelector> > _reg_trkeff_selectors;
    std::map<std::string, std::shared_ptr<TrackEfficHistos> > _reg_trkeff_histos;
    std::vector<std::string> _regions;

    std::map<std::string, std::shared_ptr<BaseSelector> > _reg_three_prong_trkeff_selectors;
    std::map<std::string, std::shared_ptr<ThreeProngHistos> > _reg_three_prong_trkeff_histos;
    std::vector<std::string> _three_prong_regions;
    

    typedef std::map<std::string,std::shared_ptr<TrackEfficHistos> >::iterator reg_it;
    typedef std::map<std::string,std::shared_ptr<ThreeProngHistos> >::iterator three_prong_reg_it;

    std::string histoCfg_{""};
    std::string thrProngCfg_{""};
    std::string cluHistoCfg_{""};
    double timeOffset_{-999};
    //In GeV. Default is 2016 value;
    double beamE_{2.3};
    int isData{0};
    std::shared_ptr<AnaHelpers> _ah;
    /*
    struct TridentCand{
	std::pair<CalCluster*,Track*> ele;
	std::pair<CalCluster*,Track*> pos;
    };
    */
    struct TridentCand{
	Particle* ele;
	Particle* pos;
    };

    struct ThreeProngCand{
	Particle* ele;
	Particle* pos;
	Particle* recoil;
    };

    //Debug level
    int debug_{0};
};

#endif
