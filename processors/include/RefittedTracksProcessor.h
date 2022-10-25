#ifndef __REFITTEDTRACKS_PROCESSOR_H__
#define __REFITTEDTRACKS_PROCESSOR_H__

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
#include "Vertex.h"
#include "Event.h"
#include "TrackHistos.h"
#include "TrackerHit.h"
#include "RawSvtHit.h"

// Forward declarations
class TTree; 

/**
 * @brief Insert description here.
 * more details
 */
class RefittedTracksProcessor : public Processor { 

    public: 

        /**
         * @brief Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        RefittedTracksProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~RefittedTracksProcessor(); 

        /**
         * @brief Process the event and put new data products into it.
         * 
         * @param event The Event to process.
         */
        virtual bool process(IEvent* ievent);

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

        /**
         * @brief description
         * 
         * @param parameters 
         */
        virtual void configure(const ParameterSet& parameters);

    private: 

        /** Container to hold all Track objects. */
	    std::vector<Track*> tracks_{};

        /** Container to hold all Track objects. */
	    std::vector<Track*> refit_tracks_{};
        
        /** Container to hold the hits on track */
	    std::vector<TrackerHit*> hits_{};

        /** Container to hold the raw hits */
        std::vector<RawSvtHit*> raw_hits_{};
        
        /** Container to hold vertex objects */
        std::vector<Vertex*> vertices_{};
        /** Container to hold refit vertex objects */
        std::vector<Vertex*> vertices_refit_{};
	
        bool _debug{false}; //!< description

        TrackHistos* _OriginalTrkHistos; //!< description
        TrackHistos* _RefitTrkHistos; //!< description
        TrackHistos* _RefitTrkHistos_chi2cut; //!< description
        TrackHistos* _RefitTrkHistos_z0cut; //!< description
        std::string histoCfg_; //!< description

}; // Refitted Tracks Processor

#endif // __REFITTEDTRACKS_PROCESSOR_
