/**
 * @file SvtHitProcessor.h
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SVTHITPROCESSOR_H
#define SVTHITPROCESSOR_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <memory> 
#include <string> 

/*~~~~~~~~~~~*/
/*   hpstr   */
/*~~~~~~~~~~~*/
#include "Processor.h" 

/// Forward declarations
class IEvent; 
class FlatTupleMaker; 
class Process; 
class TTree; 

class SvtHitProcessor : public Processor { 

    public: 

        /**
         * Constructor.
         *
         * @param name Name of the instance of this procesor.
         * @param process The Process calling this processor.
         */
        SvtHitProcessor(const std::string& name, Process& process); 

        /// Destructor 
        ~SvtHitProcessor(); 

        /**
         * Process the event and put new data products into it.
         * @param event The Event to process.
         * @return status of the processing, false will move to next event and skip other processes.
         */
        bool process(IEvent* event) final override; 

        /**
         * Callback for the Processor to take any necessary
         * action when the processing of events starts, such as
         * creating histograms.
         */
        void initialize(TTree* tree) final override; 

        /**
         * Callback for the Processor to take any necessary
         * action when the processing of events finishes, such as
         * calculating job-summary quantities.
         */
        void finalize() final override;

        /**
         * Get whether this hit is in the top half of the detector.  Modules in
         * the top half have module numbers of 0 or 2. 
         *
         * @param module The module number
         * @return True if sensor is in top layer; false if not. 
         */ 
        bool isTopLayer(int module) { return (module % 2 == 0); }

        /**
         * Get whether this hit is in the bottom half of the detector.  Modules
         * in the bottom half have module numbers of 1 or 3. 
         *
         * @param module The module number
         * @return True if the hit is in top layer; false if not. 
         */ 
        bool isBottomLayer(int module) { return (module % 2 != 0); }

        /**
         * Get whether this hit is on the electron side.  For modules in the 
         * top (bottom), the electron side has a module number of 0 (1).
         *
         * @param module The module number
         * @return True if the hit is on the electron side.
         */
        bool isElectronSide(int module); 


        /**
         * Get whether this hit is on the positron side.  For modules in the 
         * top (bottom), the electron side has a module number of 2 (3).
         *
         * @param module The module number
         * @return True if the hit is on the electron side.
         */
        bool isPositronSide(int module); 
    
    private: 
        
        /// Ntuple maker 
        std::shared_ptr< FlatTupleMaker > ntuple_;  

}; // SvtHitProcessor 

#endif // SVTHITPROCESSOR_H
