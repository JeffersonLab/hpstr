#ifndef __APV25ROXTALK_ANAPROCESSOR_H__
#define __APV25ROXTALK_ANAPROCESSOR_H__

// HPSTR
#include "Processor.h"
#include "HpsEvent.h"
#include "Collections.h"
#include "EventHeader.h"
#include "RawSvtHit.h"
#include "ModuleMapper.h"
// #include "Apv25XtalkAnaHistos.h"

// ROOT
#include "TClonesArray.h"
#include "TBranch.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TGraph.h"
#include "TTree.h"
#include "TFile.h"
#include "TString.h"

class TTree;

/**
 * @brief Insert description here.
 * more details
 */
class Apv25RoXtalkAnaProcessor : public Processor {

    public:
        /**
         * @brief Constructor
         * 
         * @param name 
         * @param process 
         */
        Apv25RoXtalkAnaProcessor(const std::string& name, Process& process);

        ~Apv25RoXtalkAnaProcessor();

        /**
         * @brief description
         * 
         * @param ievent 
         * @return true 
         * @return false 
         */
        virtual bool process(IEvent* ievent);

        /**
         * @brief description
         * 
         * @param tree 
         */
        virtual void initialize(TTree* tree);

        /**
         * @brief description
         * 
         */
        virtual void finalize();

        /**
         * @brief Configure using given parameters.
         * 
         * @param parameters The parameters used for configuration.
         */
        virtual void configure(const ParameterSet& parameters);


    private:
        /**
         * @brief description
         * 
         * @param buffIter 
         */
        void emulateApv25Buff(int buffIter);

        //Containers to hold histogrammer info
        //RecoHitAnaHistos* histos{nullptr};
        
        std::string  histCfgFilename_; //!< description
        ModuleMapper * modMap_{nullptr}; //!< description

        /** \todo Change this to be held from HPSEvent */
        TTree* tree_; //!< description
        TBranch* bevth_{nullptr}; //!< description
        TBranch* brawHits_{nullptr}; //!< description

        EventHeader* evth_{nullptr}; //!< description
        std::vector<RawSvtHit*> * rawHits_{}; //!< description

        std::string anaName_{"apvRoXtalkAna"}; //!< description
        std::string rawHitColl_{"SVTRawTrackerHits"}; //!< description
        int syncPhase_{224}; //!< description
        int trigPhase_{8}; //!< description
        int trigDel_{6696}; //!< description

        std::vector<long> reads; //!< description
        std::vector<long> readEvs; //!< description
        std::vector<long> eventTimes; //!< description
        std::vector<int>  hitMultis; //!< description
        std::vector<int>  lFEBMultis; //!< description
        double  lFEBrms[210]; //!< description
        double  lLowCut; //!< description
        double  lHighCut; //!< description
        std::vector<int>  hFEBMultis; //!< description
        double  hFEBrms[210]; //!< description
        double  hLowCut; //!< description
        double  hHighCut; //!< description
        double  sps[210]; //!< description

        int debug_{0}; //!< debug level

};


#endif
