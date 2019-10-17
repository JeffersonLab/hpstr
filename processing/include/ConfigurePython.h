/**
 * @file ConfigurePython.h
 * @brief Utility class that reads/executes a python script and creates a 
 *        Process object based on the input.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef __CONFIGURE_PYTHON_H__
#define __CONFIGURE_PYTHON_H__

//------------//
//   Python   //
//------------//
#include "Python.h"

//----------------//
//   C++ StdLib   //
//----------------//
#include <exception>
#include <iostream>
#include <stdexcept>
#include <vector>

//-----------//
//   hpstr   //
//-----------//
#include "Process.h"
#include "ProcessorFactory.h"
#include "ParameterSet.h"

class ConfigurePython {


    public:

        /**
         * Class constructor.
         *
         * This method contains all the parsing and execution of the python script.
         *
         * @param pythonScript Filename location of the python script.
         * @param args Commandline arguments to be passed to the python script.
         * @param nargs Number of commandline arguments.
         */
        ConfigurePython(const std::string& pythonScript, char* args[], int nargs);

        /**
         * Class destructor.
         */
        ~ConfigurePython();

        /** Create a process object based on the python file information. */
        Process* makeProcess();

    private: 

        /** The maximum number of events to process, if provided in python file. */
        int event_limit_{-1};

        /** List of input files to process in the job, if provided in python file. */
        std::vector<std::string> input_files_;
            
        /** List of rules for shared libraries to load, if provided in python file. */
        std::vector<std::string> libraries_;

        /** List of rules for output ROOT file names, if provided in python file. */
        std::vector<std::string> output_files_;

        /**
         * @struct ProcessorInfo
         * @brief Represents the configuration of an EventProcessor in the job.
         */
        struct ProcessorInfo {
            std::string classname_;
            std::string instancename_;
            ParameterSet params_;
        };

        /** The sequence of EventProcessor objects to be executed in order. */
        std::vector<ProcessorInfo> sequence_;

};

#endif // __CONFIGURE_PYTHON_H__
