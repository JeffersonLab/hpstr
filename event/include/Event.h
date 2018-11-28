/**
 * @file Event.h 
 * @brief Class defining methods used to access event information and data
 *        collections.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef __EVENT_H__
#define __EVENT_H__

//----------//
//   ROOT   //
//----------//
#include <TClonesArray.h>
#include <TTree.h>

class Event { 

    public: 

        /** Constructor */
        Event(); 

        /** Destructor */
        ~Event(); 

        /** 
         * Add a collection (TClonesArray) of objects to the event. 
         *
         * @param name Name of the collection
         * @param collection The TClonesArray containing the object. 
         */
        void addCollection(const std::string name, TClonesArray* collection);

        /** @return The ROOT tree containing the event. */
        TTree* getTree() { return tree_; }

    private: 

        /** The ROOT tree containing the event. */
        TTree* tree_{nullptr}; 

}; // Event

#endif // __EVENT_H__


