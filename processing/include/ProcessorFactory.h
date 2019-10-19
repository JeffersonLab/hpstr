/**
 * @file ProcessorFactory.h
 * @brief Class which provides a singleton module factory that creates Processor
 *        objects
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef __PROCESSOR_FACTORY_H__
#define __PROCESSOR_FACTORY_H__

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>

//----------//
//   ROOT   //
//----------//
#include "TObject.h"

//
#include "Processor.h"

class ProcessorFactory {

    public:

        /**
         * Get the factory instance.
         * @return The factory.
         */
        static ProcessorFactory& instance() {
            
            /** Factory for creating the Processor object. */
            static ProcessorFactory instance_;
            
            return instance_;
        }

        /**
         * Register the event processor.
         * @param classname The name of the class associated with processor.
         * @param classtype The type of class associated with processor.
         * @param maker TODO.
         */
        void registerProcessor(const std::string& classname, ProcessorMaker* maker);

        /**
         * Get the classes associated with the processor.
         * @return a vector of strings corresponding to processor classes.
         */
        //std::vector<std::string> getProcessorClasses() const;

        /**
         * Make an event processor.
         * @param classname Class name of event processor.
         * @param module_instance_name TODO.
         * @param process The process type to create.
         */
        Processor* createProcessor(const std::string& classname, const std::string& module_instance_name, Process& process);

        /**
         * Load a library.
         * @param libname The library to load.
         */
        void loadLibrary(const std::string& libname);
          
       /**
        * @struct ProcessorInfo
        * @brief Processor info container to hold classname, class type and maker.
        */
        struct ProcessorInfo {
           std::string classname;
           ProcessorMaker* maker;
        };

    private:

        /** Constructor */
        ProcessorFactory() {};

        /** A map of names to processor containers. */
        std::map<std::string, ProcessorInfo> module_info_;
        

        /** A set of names of loaded libraries. */
        std::set<std::string> libs_loaded_;


      ClassDef(ProcessorFactory,1);
  
};

#endif  // ProcessorFactory
