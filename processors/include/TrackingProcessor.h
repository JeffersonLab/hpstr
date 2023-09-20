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

/**
 * @brief Insert description here.
 * more details
 */
class TrackingProcessor : public Processor { 

    public:
        /**
         * @brief Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        TrackingProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~TrackingProcessor(); 

        /**
         * @brief Process the event and put new data products into it.
         * 
         * @param ievent The Event to process.
         */
        virtual bool process(IEvent* ievent);

        /**
         * @brief Configure the Processor
         * 
         * @param parameters The configuration parameters
         */
        virtual void configure(const ParameterSet& parameters);

        /**
         * @brief Callback for the Processor to take any necessary
         *        action when the processing of events starts.
         * 
         * @param tree
         */
        virtual void initialize(TTree* tree);

        /**
         * @brief Callback for the Processor to take any necessary
         *        action when the processing of events finishes.
         */
        virtual void finalize();

    private: 

        /** Container to hold all TrackerHit objects, and collection names. */
        std::vector<TrackerHit*> hits_{}; 
        std::string trkhitCollRoot_{"RotatedHelicalOnTrackHits"}; //!< description

        /** Container to hold all Track objects. */
        std::vector<Track*> tracks_{};
        std::string trkCollLcio_{"GBLTracks"}; //!< collection name
        std::string kinkRelCollLcio_{"GBLKinkDataRelations"}; //!< collection name
        std::string trkRelCollLcio_{"TrackDataRelations"}; //!< collection name
        std::string trkCollRoot_{"GBLTracks"}; //!< collection name

        /** Container to hold all raw hits objecs. */
        std::vector<RawSvtHit*> rawhits_{};
        std::string hitFitsCollLcio_{"SVTFittedRawTrackerHits"}; //!< collection name
        std::string rawhitCollRoot_{"SVTRawHitsOnTrack"}; //!< collection name
        
        /** Container to hold truth tracks */
        std::vector<Track*> truthTracks_{};
        std::string truthTracksCollRoot_{""}; //!< description
        std::string truthTracksCollLcio_{""}; //!< description

        //Specify trackstate location used for tracks
        std::string trackStateLocation_{""}; //!< Specify track state used for track collection DEFAULT AtIP
        
        int debug_{false}; //!< Debug Level
        
        int doResiduals_{0}; //!< do Residuals
        std::string trackResDataLcio_{""}; //!< description
        TrackHistos* trkResHistos_{nullptr}; //!< description
        std::string resCfgFilename_{""}; //!< description
        std::string resoutname_{""}; //!< description

        double bfield_{-1.}; //!< magnetic field

}; // Tracking Processor

#endif // __TRACKING_PROCESSOR_H__
