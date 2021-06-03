/**
 *
 */

#ifndef __TRACKHIT_ANAPROCESSOR_H__
#define __TRACKHIT_ANAPROCESSOR_H__

//-----------------//
//   C++  StdLib   //
//-----------------//
#include <iostream>
#include <string>
#include <vector>

//----------//
//   ROOT   //
//----------//
#include "TClonesArray.h"
#include "TRefArray.h"

//-----------//
//   hpstr   //
//-----------//
#include "Processor.h"
#include "BaseSelector.h"
#include "Track.h"
#include "TrackerHit.h"
#include "Event.h"
#include "TrackHistos.h"
#include "AnaHelpers.h"

// Forward declarations
class TTree; 

class TrackHitAnaProcessor : public Processor { 

    public: 

        /**
         * Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        TrackHitAnaProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~TrackHitAnaProcessor(); 

        /**
         * Configure the Ana Processor
         * @param parameters The configuration parameters
         */
        virtual void configure(const ParameterSet& parameters);

        /**
         * Process the event and put new data products into it.
         * @param event The Event to process.
         */
        virtual bool process(IEvent* ievent);

        /**
         * Callback for the Processor to take any necessary
         * action when the processing of events starts.
         */
        virtual void initialize(TTree* tree);

        /**
         * Callback for the Processor to take any necessary
         * action when the processing of events finishes.
         */
        virtual void finalize();

    private: 

        /** Container to hold all Track objects. */
        std::vector<Track*> * tracks_{};
        TBranch*           btracks_{nullptr};

        // Track Collection name
        std::string trkCollName_;

        // Track Selector configuration
        std::string selectionCfg_;
        std::shared_ptr<BaseSelector> trkSelector_;
        std::vector<std::string> regionSelections_;

        std::map<std::string, std::shared_ptr<BaseSelector> > reg_selectors_;

        std::map<std::string, std::shared_ptr<TrackHistos> > reg_histos_;
        typedef std::map<std::string,std::shared_ptr<TrackHistos> >::iterator reg_it;

        std::vector<std::string> regions_;

        // Containers to hold histogrammer info
        std::string histCfgFilename_;
        std::string truthHistCfgFilename_;
        TrackHistos* trkHistos_{nullptr};
        TrackHistos* truthHistos_{nullptr};
        bool doTruth_{false};
        int debug_{0};
        



}; // TrackHitAnaProcessor

#endif // __TRACKHIT_ANAPROCESSOR_
