#ifndef FINDABLETRACKPROCESSOR_H
#define FINDABLETRACKPROCESSOR_H

/*~~~~~~~~~~~*/
/*   hpstr   */
/*~~~~~~~~~~~*/
#include "Processor.h"

/*~~~~~~~~~~*/
/*   ROOT   */
/*~~~~~~~~~~*/
#include "TBranch.h" 

/// Forward declarations
class FindableTrack;
class IEvent;
class MCParticle;
class MCTrackerHit; 
class Process; 
class TTree;

class FindableTrackProcessor : public Processor { 

    public: 

        /**
         * Constructor.
         *
         * @param name Name of the instance of this procesor.
         * @param process The Process calling this processor.
         */
        FindableTrackProcessor(const std::string& name, Process& process); 

        /// Destructor 
        ~FindableTrackProcessor();

        /**
         *
         */
        void configure(const ParameterSet& parameters);  

        /**
         * The main processing loop.  
         *
         * @param event The Event to process.
         * @return Status of the processing. If false, the event will be 
         *         immediately skipped along with the rest of the processes in 
         *         the event.
         */
        bool process(IEvent* event) final override; 

        /**
         * Callback for the Processor to take any necessary action when the 
         * processing of events starts, such as creating histograms.
         */
        void initialize(TTree* tree) final override; 

        /**
         * Callback for the Processor to take any necessary action when the 
         * processing of events finishes, such as calculating job-summary 
         * quantities.
         */
        void finalize() final override;

        /**
         * Create a hit map which associates a charged particle to it's hits 
         * in the tracker. 
         *
         * @param[in] hits collection of simulated tracker hits.
         */
        void createHitMap(const std::vector< MCTrackerHit* >*  hits);

        /**
         * Check whether a track is findable or not.
         *
         * @param[in] hit_count The total number of hits composing the track.  
         */
        void isFindable(int lcio_id, std::vector< int > hit_count); 
       
    private:

        ///
        void clear();          

        /// Name of the collection of MC particles
        std::string mc_particle_col_{""};

        /// Name of the collection of Simulated tracker hits
        std::string sim_tracker_hit_col_{""};  

        /// Container for MC particles
        std::map<int, MCParticle*>* mc_particles_{nullptr};

        /// Container for Simulated tracker hits
        std::vector<MCTrackerHit*>* sim_tracker_hits_{nullptr};  

        /// Map between a sim particle and its hit array
        std::map< int, std::vector< int >> hitMap_; 

        /// Collection of findable tracks
        std::vector< FindableTrack* >* findable_tracks_;  

}; // FindableTrackProcessor 

#endif // FINDABLETRACKPROCESSOR_H
