#ifndef __SVTBL2D_ANAPROCESSOR_H__
#define __SVTBL2D_ANAPROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "RawSvtHit.h"
#include "ModuleMapper.h"
#include "TSData.h"

//ROOT
#include "Svt2DBlHistos.h"
#include "Processor.h"
#include "TClonesArray.h"
#include "TFile.h"
#include "json.hpp"

class TTree;

/**
 * @brief Insert description here.
 * more details
 */
class SvtBl2DAnaProcessor : public Processor {

    public:
        /**
         * @brief Constructor
         * 
         * @param name 
         * @param process 
         */
        SvtBl2DAnaProcessor(const std::string& name, Process& process);

        ~SvtBl2DAnaProcessor();

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
         * @brief description
         * 
         * @param parameters 
         */
        virtual void configure(const ParameterSet& parameters);


    private:

        int Event_number = 0; //!< description
        Svt2DBlHistos* svtCondHistos{nullptr}; //!< description

        ModuleMapper* mmapper_; //!< Initialize ModuleMapper

        std::string histCfgFilename_; //!< configuration parameters
        std::string rawSvtHitsColl_{"SVTRawTrackerHits"}; //!< description
        std::vector<RawSvtHit*>* rawSvtHits_{}; //!< description
        TBranch* brawSvtHits_{nullptr}; //!< description
        TTree* tree_; //!< description

        std::string triggerFilename_; //!< trigger selection
        json        triggers_; //!< description
        std::string triggerBankColl_{"TSBank"}; //!< description

        TBranch* btriggerBank_{nullptr}; //!< description
        TObject* triggerBank_{}; //!< description
        std::map<std::string, bool> prescaledtriggerMap_; //!< description
        std::map<std::string, bool> exttriggerMap_; //!< description

        int debug_{0}; //!< Debug level

};


#endif
