#ifndef __VERTEX_ANAPROCESSOR_H__
#define __VERTEX_ANAPROCESSOR_H__


//HPSTR
#include "HpsEvent.h"
#include "Collections.h"
#include "EventHeader.h"
#include "TSData.h"
#include "Vertex.h"
#include "Track.h"
#include "TrackerHit.h"
#include "MCParticle.h"
#include "Particle.h"
#include "Processor.h"
#include "BaseSelector.h"
#include "TrackHistos.h"
#include "MCAnaHistos.h"


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

struct char_cmp {
    bool operator () (const char *a,const char *b) const
    {
        return strcmp(a,b)<0;
    }
};


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
        std::map<const char *, int, char_cmp> brMap_;
        TBranch* bts_{nullptr};
        TBranch* bvtxs_{nullptr};
        TBranch* bhits_{nullptr};
        TBranch* btrks_{nullptr};
        TBranch* bmcParts_{nullptr};
        TBranch* bevth_{nullptr};
        TBranch* becal_{nullptr};

        EventHeader * evth_{nullptr};
        TSData      * ts_{nullptr};
        std::vector<CalCluster*> * ecal_{};
        std::vector<Vertex*> * vtxs_{};
        std::vector<Track*>  * trks_{};
        std::vector<TrackerHit*>  * hits_{};
        std::vector<MCParticle*>  * mcParts_{};

        std::string anaName_{"vtxAna"};
        std::string tsColl_{"TSBank"};
        std::string vtxColl_{"Vertices"};
        std::string hitColl_{"RotatedHelicalTrackHits"};
        std::string trkColl_{"GBLTracks"};
        std::string ecalColl_{"RecoEcalClusters"};
        std::string mcColl_{"MCParticle"};
        int isRadPDG_{622};
        TTree* tree_{nullptr};

        std::shared_ptr<TrackHistos> _vtx_histos;
        std::shared_ptr<MCAnaHistos> _mc_vtx_histos;

        //Duplicate.. We can make a single class.. ?
        std::map<std::string, std::shared_ptr<BaseSelector> > _reg_vtx_selectors;
        std::map<std::string, std::shared_ptr<TrackHistos> > _reg_vtx_histos;
        std::map<std::string, std::shared_ptr<MCAnaHistos> > _reg_mc_vtx_histos;
        std::map<std::string, std::shared_ptr<FlatTupleMaker> > _reg_tuples;

        std::vector<std::string> _regions;

        typedef std::map<std::string,std::shared_ptr<TrackHistos> >::iterator reg_it;
        typedef std::map<std::string,std::shared_ptr<MCAnaHistos> >::iterator reg_mc_it;

        std::string histoCfg_{""};
        std::string mcHistoCfg_{""};
        double timeOffset_{-999};
        //In GeV. Default is 2016 value;
        double beamE_{2.3};
        int isData_{0};
        std::string analysis_{"vertex"};

        std::shared_ptr<AnaHelpers> _ah;

        //Debug level
        int debug_{0};
};

#endif
