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

    private: 
        
        /// Ntuple maker 
        std::shared_ptr< FlatTupleMaker > ntuple_;  

}; // SvtHitProcessor 

#endif // SVTHITPROCESSOR_H
