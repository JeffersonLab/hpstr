/**
 * @file Processor.h
 * @brief Base classes for all user event processing components to extend
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef __PROCESSOR_H__
#define __PROCESSOR_H__

//----------------//
//   C++ StdLib   //
//----------------//
#include <map>

//-----------//
//   hpstr   //
//-----------//
#include "ParameterSet.h"

// Forward declarations
class Process;
class Processor;
class TTree;
class TFile;
class IEvent;

/** Typedef for ProcessorFactory use. */
typedef Processor* ProcessorMaker(const std::string& name, Process& process);

/**
 * @class Processor
 * @brief Base class for all event processing components
 */
class Processor {

    public:

        /**
         * Class constructor.
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided by the framework.
         *
         * @note The name provided to this function should not be
         * the class name, but rather a logical label for this instance of
         * the class, as more than one copy of a given class can be loaded
         * into a Process with different parameters.  Names should not include
         * whitespace or special characters.
         */
        Processor(const std::string& name, Process& process);

        /**
         * Class destructor.
         */
        virtual ~Processor() {;}

        /**
         * Callback for the Processor to configure itself from the given set of parameters.
         * @param parameters ParameterSet for configuration.
         */
        virtual void configure(const ParameterSet& parameters) {
        }

        /**
         * Callback for the Processor to take any necessary
         * action when the processing of events starts, such as
         * creating histograms.
         */
        virtual void initialize(TTree* tree) = 0;

        /**
         * Set output TFile for AnaProcessors
         * @param pointer to output TFile
         */
        virtual void setFile(TFile* outFile) {outF_ = outFile;};

        /**
         * Process the event and put new data products into it.
         * @param event The Event to process.
         * @return status of the processing, false will move to next event and skip other processes.
         */
        virtual bool process(IEvent* ievent) = 0;

        /**
         * Callback for the Processor to take any necessary
         * action when the processing of events finishes, such as
         * calculating job-summary quantities.
         */
        virtual void finalize()  = 0; 

        /**
         * Internal function which is part of the ProcessorFactory machinery.
         * @param classname The class name of the processor.
         * @param classtype The class type of the processor (1 for Producer, 2 for Analyzer).
         */
        static void declare(const std::string& classname, ProcessorMaker*);

    protected:

        /** Handle to the Process. */
        Process& process_;

        /** output file pointer */
        TFile* outF_{nullptr};

    private:

        /** The name of the Processor. */
        std::string name_;


};

/**
 * @def DECLARE_PROCESSOR(CLASS)
 * @param CLASS The name of the class to register, which must not be in a namespace.
 * @brief Macro which allows the framework to construct a producer given its name during configuration.
 * @attention Every Producer class must call this macro in the associated implementation (.cxx) file.
 */
#define DECLARE_PROCESSOR(CLASS) Processor* CLASS ## _make (const std::string& name, Process& process) { return new CLASS(name,process); }  __attribute__((constructor(1000))) static void CLASS ## _declare() { Processor::declare(#CLASS, & CLASS ## _make ); }

#endif
