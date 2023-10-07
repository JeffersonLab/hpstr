#ifndef __SVTTIMING_PROCESSOR_H__
#define __SVTTIMING_PROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "Collections.h"
#include "Track.h"
#include "TrackerHit.h"
#include "SvtTimingHistos.h"
#include "BaseSelector.h"
#include "EventHeader.h"
#include "AnaHelpers.h"
#include "ModuleMapper.h"
//ROOT
#include "Processor.h"
#include "TClonesArray.h"
#include "TBranch.h"
#include "TTree.h"
#include "TFile.h"

class TTree;


class SvtTimingProcessor : public Processor {

    public:

        SvtTimingProcessor(const std::string& name, Process& process);

        ~SvtTimingProcessor();

        virtual bool process(IEvent* ievent);

        virtual void initialize(TTree* tree);

        virtual void finalize();

        virtual void configure(const ParameterSet& parameters);

    private:
        ModuleMapper * mmapper_;
        //Containers to hold histogrammer info
        SvtTimingHistos* histos{nullptr};
        std::string  histCfgFilename_;
        int runNumber_;
        //TODO Change this to be held from HPSEvent
        TTree* tree_;
        TBranch* brawHits_{nullptr};
        TBranch* btrkrHits_{nullptr};
        TBranch* btracks_{nullptr};
        TBranch* bfsps_{nullptr};
        TBranch* becalHits_{nullptr};
        TBranch* becalClusters_{nullptr};
        TBranch* bevth_{nullptr};

        std::vector<RawSvtHit*> * rawHits_{};
        std::vector<TrackerHit*> * trkrHits_{};
        std::vector<Track*>      * tracks_{};
        std::vector<Particle*>      * fsps_{};
        std::vector<CalHit*>     * ecalHits_{};
        std::vector<CalCluster*> * ecalClusters_{};
        EventHeader* evth_{nullptr};

        std::string anaName_{"recoHitAna"};
        std::string trkColl_{"GBLTracks"};
        std::string rawHitColl_{"SVTRawHitsOnTrack_KF"};
        std::string trkrHitColl_{"RotatedHelicalTrackHits"};
        std::string fspColl_{"FinalStateParticles_KF"};
        std::string ecalHitColl_{"EcalCalHits"};
        std::string ecalClusColl_{"EcalClustersCorr"};
        std::string timingCalibDir_{"timingCalibDir"};
        std::string postfixTiming_{"postfixTiming"};
        
        std::shared_ptr<BaseSelector> evtSelector_;
        std::string selectionCfg_;
        
        std::vector<std::string> regionSelections_;
        std::map<std::string, std::shared_ptr<BaseSelector> > _reg_phase_selectors;
        std::map<std::string, std::shared_ptr<SvtTimingHistos> > _reg_phase_histos;
        std::vector<std::string> _regions;

        typedef std::map<std::string,std::shared_ptr<SvtTimingHistos> >::iterator reg_it;
        std::shared_ptr<AnaHelpers> _ah;
        std::map<std::string,double> timingCalibConstants_;
        //Debug Level
        int debug_{0};



};


#endif
