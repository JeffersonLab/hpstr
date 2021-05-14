#ifndef __TRIGGERPARAMETERSEXTRACTION_ANAPROCESSOR_H__
#define __TRIGGERPARAMETERSEXTRACTION_ANAPROCESSOR_H__


//HPSTR
#include "HpsEvent.h"
#include "Collections.h"
#include "Track.h"
#include "CalCluster.h"
#include "CalHit.h"
#include "Processor.h"
#include "HistoManager.h"
#include "TriggerParametersExtractionAnaHistos.h"

#include "BaseSelector.h"

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


class TriggerParametersExtractionAnaProcessor : public Processor {

    public:
		TriggerParametersExtractionAnaProcessor(const std::string& name, Process& process);
        ~TriggerParametersExtractionAnaProcessor();
        virtual bool process(IEvent* ievent);

        virtual void initialize(TTree* tree);

        virtual void finalize();

        virtual void configure(const ParameterSet& parameters);

    private:
        //Containers to hold histogrammer info
        TriggerParametersExtractionAnaHistos* histos{nullptr};
        std::string  histCfgFilename_;

        TTree* tree_{nullptr};
        TBranch* btrks_{nullptr};
        TBranch* becalClusters_{nullptr};

        std::vector<Track*>  * trks_{};
        std::vector<CalCluster*> * ecalClusters_{};

        std::string anaName_{"vtxAna"};
        std::string trkColl_{"GBLTracks"};
        std::string ecalClusColl_{"EcalClustersCorr"};


        double beamE_{3.7};
        int isData_{0};
        std::string analysis_{"triggerParametersExtraction"};

        //Debug level
        int debug_{0};
};

#endif
