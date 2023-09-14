#ifndef __TRIDENTWAB_ANAPROCESSOR_H__
#define __TRIDENTWAB_ANAPROCESSOR_H__


//HPSTR
#include "HpsEvent.h"
#include "Collections.h"
#include "EventHeader.h"
#include "Vertex.h"
#include "Track.h"
#include "TrackerHit.h"
#include "RawSvtHit.h"
#include "Particle.h"
#include "Processor.h"
#include "BaseSelector.h"
#include "TridentHistos.h"
#include "FlatTupleMaker.h"
#include "AnaHelpers.h"
#include "MCParticle.h"
#include "TSData.h"
//ROOT
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TVector3.h"

//C++ 
#include <memory>
#include  <random>
#include  <iterator>
class TridentWABAnaProcessor : public Processor {
    
public:
    TridentWABAnaProcessor(const std::string& name, Process& process);
    ~TridentWABAnaProcessor();
    virtual bool process(IEvent* ievent);

    virtual void initialize(TTree* tree);
    
    virtual void finalize();
    
    virtual void configure(const ParameterSet& parameters);

private:
    
    std::shared_ptr<BaseSelector> vtxSelector;
    std::shared_ptr<BaseSelector> trkSelector;
    std::vector<std::string> regionSelections_;
    std::vector<std::string> regionWABSelections_;
    
    std::string selectionCfg_;
    std::string trkSelCfg_;


    TBranch* bfspart_{nullptr};
    TBranch* bvtxs_{nullptr};
    TBranch* btrks_{nullptr};
    TBranch* bhits_{nullptr};
    TBranch* brawhits_{nullptr};
    TBranch* bclus_{nullptr};
    TBranch* btsdata_{nullptr};
    TBranch* bmcParts_{nullptr};
    TBranch* bevth_{nullptr};
    
    std::vector<Vertex*> * vtxs_{};
    std::vector<TrackerHit*> * hits_{};
    std::vector<RawSvtHit*> * rawhits_{};
    std::vector<Particle*> * fspart_{};
    std::vector<Track*>  * trks_{};
    std::vector<CalCluster*>  * clus_{};
    std::vector<MCParticle*> * mcParts_{};
    EventHeader* evth_{nullptr};
    TSData* tsdata_{nullptr};
    
    std::string anaName_{"vtxAna"};
    std::string mcColl_{"MCParticle"};
    std::string hitColl_{"SiClustersOnTrack"};
    std::string rawhitColl_{"TrackRawHits"};
    std::string cluColl_{"Vertices"};
    std::string vtxColl_{"Vertices"};
    std::string tsdataColl_{"TSData"};
    std::string fspartColl_{"FinalStateParticles_KF"};
    std::string trkColl_{"GBLTracks"};
    TTree* tree_{nullptr};

    std::shared_ptr<TridentHistos> _vtx_histos;
    
    //Duplicate.. We can make a single class.. ?
    std::map<std::string, std::shared_ptr<BaseSelector> > _reg_vtx_selectors;
    std::map<std::string, std::shared_ptr<TridentHistos> > _reg_vtx_histos;
    std::map<std::string, std::shared_ptr<FlatTupleMaker> > _reg_tuples;

    std::map<std::string, std::shared_ptr<BaseSelector> > _reg_WAB_selectors;
    std::map<std::string, std::shared_ptr<TridentHistos> > _reg_WAB_histos; 
    
    std::vector<std::string> _regions;
    std::vector<std::string> _regionsWAB;
    

    typedef std::map<std::string,std::shared_ptr<TridentHistos> >::iterator reg_it;

    std::string histoCfg_{""};
    double calTimeOffset_{-999};
    double trkTimeOffset_{-999};
    //In GeV. Default is 2016 value;
    double beamE_{2.3};
    int isData{0};
    std::shared_ptr<AnaHelpers> _ah;
    Vertex* matchPairToVertex(Track* ele,Track* pos, std::vector<Vertex*>& verts);
    std::pair<Track*,MCParticle*> matchToMCParticle(Track* part, std::vector<MCParticle*>& mcParts);

    std::vector<CalCluster*> getUnmatchedClusters(std::vector<CalCluster*>& allClusters,std::vector<std::pair<CalCluster*,Track*> > electrons, std::vector<std::pair<CalCluster*,Track*> > positrons);

    struct TridentCand{
	Vertex* vertex; 
        CalCluster* eleClu;
	CalCluster* posClu;
    };

   struct WABCand{
	Track* eleTrk; 
        CalCluster* eleClu;
	CalCluster* posClu;
    };

   
   template<typename Iter, typename RandomGenerator>
       Iter select_randomly(Iter start, Iter end, RandomGenerator& g) {
       std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
       std::advance(start, dis(g));
       return start;
   }
   
   template<typename Iter>
       Iter select_randomly(Iter start, Iter end) {
       static std::random_device rd;
       static std::mt19937 gen(rd());
       return select_randomly(start, end, gen);
   }
   
    //Debug level
    int debug_{0};
};

#endif
