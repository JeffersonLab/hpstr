#ifndef __VERTEX_PROCESSOR_H__
#define __VERTEX_PROCESSOR_H__

//-----------------//
//   C++  StdLib   //
//-----------------//
#include <iostream>
#include <string>
#include <vector>

//----------//
//   LCIO   //
//----------//
#include <EVENT/LCCollection.h>
#include <EVENT/ReconstructedParticle.h>
#include <IMPL/LCGenericObjectImpl.h>
#include <UTIL/LCRelationNavigator.h>

//-----------//
//   hpstr   //
//-----------//
#include "Processor.h"
#include "Vertex.h"
#include "Particle.h"
#include "Event.h"
#include "TrackerHit.h"
#include "RawSvtHit.h"

// Forward declarations
class TTree; 

/**
 * @brief Insert description here.
 * more details
 */
class VertexProcessor : public Processor { 

    public: 
        /**
         * @brief Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        VertexProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~VertexProcessor(); 

        /**
         * @brief Callback for the Processor to configure itself from the given set of parameters.
         * 
         * @param parameters ParameterSet for configuration.
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
         * @brief Process the event and put new data products into it.
         * 
         * @param ievent The Event to process.
         */
        virtual bool process(IEvent* ievent);

        /**
         * @brief Callback for the Processor to take any necessary
         *        action when the processing of events finishes.
         */
        virtual void finalize();

    private: 

        /** Containers to hold all TrackerHit objects. */
        std::vector<TrackerHit*> hits_{};
        std::string trkhitCollRoot_{"fspOnTrackHits"};
        std::vector<RawSvtHit*> rawhits_{};
        std::string rawhitCollRoot_{"fspOnTrackRawHits"};
        std::vector<Vertex*> vtxs_{}; //!< description
        std::vector<Particle*> parts_{}; //!< description
        std::string vtxCollLcio_{"UnconstrainedV0Vertices"}; //!< description
        std::string vtxCollRoot_{"UnconstrainedV0Vertices"}; //!< description
        std::string partCollRoot_{"ParticlesOnVertices"}; //!< description
        std::string kinkRelCollLcio_{"GBLKinkDataRelations"}; //!< description
        std::string trkRelCollLcio_{"TrackDataRelations"}; //!< description
        std::string hitFitsCollLcio_{"SVTFittedRawTrackerHits"};
        std::string trackStateLocation_{""}; //!< select track state for tracks DEFAULT AtIP

        double bfield_{-1.}; //!< magnetic field

        int debug_{0}; //!< Debug Level

}; // VertexProcessor

#endif // __VERTEX_PROCESSOR_H__
