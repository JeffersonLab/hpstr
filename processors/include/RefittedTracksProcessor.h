/**
 *
 */

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

class RefittedTracksProcessor : public Processor { 

    public: 

        /**
         * Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        RefittedTracksProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~RefittedTracksProcessor(); 

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
	std::vector<Track*> tracks_{};

        /** Container to hold all Track objects. */
	std::vector<Track*> refit_tracks_{};
        
        /** Container to hold the hits on track */
	std::vector<TrackerHit*> hits_{};

	/** Container to hold the raw hits */
	std::vector<RawSvtHit*> raw_hits_{};
	
	/** Container to hold vertex objects */
	std::vector<Vertex*> vertices_{};
	std::vector<Vertex*> vertices_refit_{};
	

	bool _debug{false};

	TrackHistos* _OriginalTrkHistos;
	TrackHistos* _RefitTrkHistos;
	TrackHistos* _RefitTrkHistos_chi2cut;
	TrackHistos* _RefitTrkHistos_z0cut;
        


}; // Refitted Tracks Processor

#endif // __REFITTEDTRACKS_PROCESSOR_
