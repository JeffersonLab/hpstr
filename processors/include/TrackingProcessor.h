/**
 *
 */

#ifndef __TRACKING_PROCESSOR_H__
#define __TRACKING_PROCESSOR_H__

//-----------------//
//   C++  StdLib   //
//-----------------//
#include <iostream>
#include <string>

//----------//
//   LCIO   //
//----------//
#include <EVENT/LCCollection.h>
#include <EVENT/Track.h>
#include <EVENT/TrackerHit.h>
#include <EVENT/TrackState.h>
#include <EVENT/TrackerRawData.h>
#include <IMPL/LCGenericObjectImpl.h>
#include <IMPL/TrackerHitImpl.h>
#include <UTIL/LCRelationNavigator.h>
#include <UTIL/BitField64.h>

//----------//
//   ROOT   //
//----------//
#include "TClonesArray.h"

//-----------//
//   hpstr   //
//-----------//
#include "Collections.h"
#include "Processor.h"
#include "Track.h"
#include "TrackerHit.h"
#include "Event.h"
#include "RawSvtHit.h"
#include "TrackHistos.h"



// Forward declarations
class TTree; 

class TrackingProcessor : public Processor { 

    public: 

        /**
         * Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        TrackingProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~TrackingProcessor(); 

        /**
         * Process the event and put new data products into it.
         * @param event The Event to process.
         */
        virtual bool process(IEvent* ievent);

        /**
         * Configure the Processor
         * @param parameters The configuration parameters
         */
        virtual void configure(const ParameterSet& parameters);

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

        /** Container to hold all TrackerHit objects, and collection names. */
        std::vector<TrackerHit*> hits_{}; 
        std::string trkhitCollRoot_{"RotatedHelicalOnTrackHits"};

        /** Container to hold all Track objects, and collection names. */
        std::vector<Track*> tracks_{};
        std::string trkCollLcio_{"GBLTracks"};
        std::string kinkRelCollLcio_{"GBLKinkDataRelations"};
        std::string trkRelCollLcio_{"TrackDataRelations"};
        std::string trkCollRoot_{"GBLTracks"};

        /** Container to hold all raw hits objecs, and collection names. */
        std::vector<RawSvtHit*> rawhits_{};
        std::string hitFitsCollLcio_{"SVTFittedRawTrackerHits"};
        std::string rawhitCollRoot_{"SVTRawHitsOnTrack"};
        
        /** Container to hold truth tracks */
        std::vector<Track*> truthTracks_{};
        std::string truthTracksCollRoot_{""};
        std::string truthTracksCollLcio_{""};
        
        //Debug Level
        int debug_{false};
        
        //do Residuals
        int doResiduals_{0};
        std::string trackResDataLcio_{""};
        TrackHistos* trkResHistos_{nullptr};
        std::string resCfgFilename_{""};
        std::string resoutname_{""};
        
        
        //Bfield
        double bfield_{-1.};




}; // Tracking Processor

#endif // __TRACKING_PROCESSOR_H__
