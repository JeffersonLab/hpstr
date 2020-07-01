#ifndef FINDABLE_TRACK_H
#define FINDABLE_TRACK_H

/*~~~~~~~~~~*/
/*   ROOT   */
/*~~~~~~~~~~*/
#include "TObject.h" 

class FindableTrack : public TObject {

    public: 
        
        /// Default constructor
        FindableTrack() = default; 

        /**
         * Constructor
         *
         * @param[in] lcio_id The LCIO ID associated with the MC particle that
         *      created this track. 
         * @param[in] hit_count The total number of stereo hits composing this 
         *      track. 
         */
        FindableTrack(int lcio_id, int hit_count, bool is_findable);

        /// Destructor 
        ~FindableTrack();

        /// Print a string representation of this object
        void Print(Option_t *option="") const; 

        /**
         * Get the number of stereo hits that this track contains.
         *
         * @return Total number of stereo hits composing this track.
         */
        int getHitCount() const { return hit_count_; };

        /**
         * Get the LCIO ID of the particle that created this track.
         *
         * @return The LCIO ID.
         */
        int getLcioID() const { return lcio_id_; }

        /**
         * Checks if this track is findable.
         *
         * @returns true if the track should be found by a tracking algorithm. 
         */
        bool isFindable() const { return is_findable_; }


       ClassDef(FindableTrack, 1); 

    private: 

        /// The total number of stereo hits composing this track.
        int hit_count_{0}; 
        
        /// The LCIO ID identifying the MC particle that created this track. 
        int lcio_id_{0}; 

        /// Flag denoting if this track is findable
        bool is_findable_{false};  

}; // FindableTrack

#endif // FINDABLE_TRACK_H 
