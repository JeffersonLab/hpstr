#ifndef MCTRACKERHIT_PROCESSOR_H
#define MCTRACKERHIT_PROCESSOR_H

//-----------------//
//   C++  StdLib   //
//-----------------//
#include <string>

//----------//
//   LCIO   //
//----------//
#include <UTIL/BitField64.h>

//-----------//
//   hpstr   //
//-----------//
#include "Processor.h"

// Forward declarations
class Event;
class MCParticle; 
class MCTrackerHit; 
class TTree;

class MCTrackerHitProcessor : public Processor { 

    public: 

        /**
         * Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        MCTrackerHitProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~MCTrackerHitProcessor(); 

        /**
         * Callback for the Processor to configure itself from the given set of parameters.
         * @param parameters ParameterSet for configuration.
         */
        virtual void configure(const ParameterSet& parameters) final override;

        /**
         * Callback for the Processor to take any necessary
         * action when the processing of events starts.
         */
        virtual void initialize(TTree* tree) final override;

        /**
         * Process the event and put new data products into it.
         * @param event The Event to process.
         */
        virtual bool process(IEvent* ievent) final override;

        /**
         * Callback for the Processor to take any necessary
         * action when the processing of events finishes.
         */
        virtual void finalize() final override;

    private: 

        /// Clear the collection of tracker hits
        void clear(); 

        /// Container for MC tracker hits 
        std::vector<MCTrackerHit*>* sim_tracker_hits_{nullptr};  
       
        /// LCIO sim tracker hit collection
        std::string sim_tracker_hit_col_{"TrackerHits"};

        /// Output collection name
        std::string mc_tracker_hit_col_{"TrackerHits"};

        /// 
        UTIL::BitField64 decoder_{"system:0:6,barrel:6:3,layer:9:4,module:13:12,sensor:25:1,side:32:-2,strip:34:12"};
        
        //Debug Level
        int debug_{0};

}; // MCTrackerHitProcessor

#endif // __MCTRACKERHIT_PROCESSOR_H__
