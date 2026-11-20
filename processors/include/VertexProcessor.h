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
#include <EVENT/TrackerHit.h>
#include <IMPL/LCGenericObjectImpl.h>
#include <EVENT/ReconstructedParticle.h>
#include <IMPL/LCGenericObjectImpl.h>
#include <UTIL/LCRelationNavigator.h>
#include <IMPL/MCParticleImpl.h>
#include <IMPL/SimTrackerHitImpl.h>
#include <EVENT/MCParticle.h>
#include <EVENT/SimTrackerHit.h>

#include <map>
#include <set>

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

        /**
         * @brief Closure test of the truth-link chain, off by default.
         *
         * The chain that feeds PreselectAndCategorize2021's truth_ele_id is
         *
         *   tracker hit -> getRawHits() -> mcPartRelLcio_ -> SimTrackerHit
         *               -> getMCParticle()->id() -> Track::addMcpHit()
         *               -> plurality -> lookup in the hpstr MCParticle collection
         *
         * and every stage can drop the link while looking identical downstream:
         * hasTruthMatch() returns false whether the relation was missing, the
         * MCParticle pointer was null, the id resolves to nothing in the stored
         * MCParticle collection, or the plurality lands on a non-e+-.  These
         * counters separate those cases and check that what LCIO offered is
         * exactly what ended up on the hpstr Track.
         */
        struct TruthLinkClosure {
            long n_events{0};              //!< events seen
            long n_events_with_rel{0};     //!< events where mcPartRelLcio_ was present
            long n_events_with_mcp{0};     //!< events where mcPartCollLcio_ was present

            long n_raw_event{0};           //!< raw hits in rawHitCollLcio_, whole event
            long n_raw_event_with_rel{0};  //!< ... having >=1 relation

            long n_tracks{0};              //!< tracks converted
            long n_trackerhits{0};         //!< tracker (cluster/3D) hits on those tracks
            long n_raw{0};                 //!< raw hits reached through those tracker hits
            long n_raw_with_rel{0};        //!< ... having >=1 relation
            long n_simhits{0};             //!< SimTrackerHits reached
            long n_simhits_null_mcp{0};    //!< ... whose getMCParticle() was null
            long n_simhits_dangling{0};    //!< ... whose MCParticle id is absent from mcPartCollLcio_

            long n_expected_entries{0};    //!< addMcpHit() calls the LCIO walk should produce
            long n_stored_entries{0};      //!< entries actually on Track::getMcpHits()

            long n_trk_no_entry{0};        //!< tracks with an empty mcp-hit list
            long n_trk_dangling{0};        //!< tracks whose plurality id is not in the MCParticle coll
            long n_trk_not_electron{0};    //!< tracks whose plurality id resolves to |PDG| != 11
            long n_trk_electron{0};        //!< tracks that resolve to an e+-
            long n_trk_plurality_differs{0};  //!< strip-level and layer-unique plurality disagree
        };

        /**
         * @brief Tally the closure counters for one converted track.
         *
         * @param track       the hpstr Track that addMcpHit() was just called on
         * @param mcp_pdg     id -> PDG for every particle in mcPartCollLcio_
         * @param have_mcp    whether mcPartCollLcio_ was present this event
         */
        void closeTruthLink(Track* track, const std::map<int, int>& mcp_pdg, bool have_mcp);

        /** @brief Print the closure summary. Called from finalize(). */
        void reportTruthLinkClosure() const;

        TruthLinkClosure closure_{};

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
        std::string mcPartRelLcio_{"RotatedHelicalTrackMCRelations"};
        std::string mcPartCollLcio_{"MCParticle"}; //!< collection the stored MCParticle ids come from
        std::string rawHitCollLcio_{"SVTRawTrackerHits"}; //!< event-level raw hits, for the closure control
        std::string trackStateLocation_{""}; //!< select track state for tracks DEFAULT AtIP

        int truthClosure_{0}; //!< Run the truth-link closure test and print a summary in finalize()

        double bfield_{-1.}; //!< magnetic field

        int useTrackerHits_{1}; //!< Load hit collections, otherwise get from getSubdetectorHitNumbers

        int debug_{0}; //!< Debug Level

}; // VertexProcessor

#endif // __VERTEX_PROCESSOR_H__
