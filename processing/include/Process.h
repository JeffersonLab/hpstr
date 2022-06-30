/**
 * @file Process.h
 * @brief Class which represents the process under execution.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */

#ifndef __PROCESS_H__
#define __PROCESS_H__

//----------------//
//   C++ StdLib   //
//----------------//
#include <vector>
#include <iostream>
#include <stdexcept>

//----------//
//   ROOT   //
//----------//
#include "TObject.h"
#include "TFile.h"

//-----------//
//   hpstr   //
//-----------//
#include "Processor.h"


class Process {

    public:

        /**
         * Class constructor.
         * @param passname Processing pass label
         */
        Process();

        /**
         * Add an event processor to the linear sequence of processors to run in this job
         * @param event_proc Processor to add to the sequence
         */
        void addToSequence(Processor* event_proc);

        /**
         * Add an input file name to the list.
         * @param filename Input ROOT event file name
         */
        void addFileToProcess(const std::string& filename);

        /**
         * Add an output file name to the list.  There should either be the same number 
         * of output file names as input file names or just one output file name.
         * @param output_filename Output ROOT event file name
         */
        void addOutputFileName(const std::string& output_filename);

        /**
         * Set the run mode of the process.
         * @param run_mode Maximum number of events to process.  
         *     0 indicates LCIO to ROOT.
         *     1 indicates ROOT to Histo.
         *     2 indicates Histo Analysis.
         */
        void setRunMode(int run_mode=-1) {
            run_mode_ = run_mode;
        }

        /**
         * Set the number of events to skip.  Processing will stop 
         * when either there are no more input events or when this number of events have been processed.
         * @param skip_events Number of events to skip.
         */
        void setSkipEvents(int skip_events=-1) {
            skip_events_ = skip_events;
        }

        /**
         * Set the maximum number of events to process.  Processing will stop 
         * when either there are no more input events or when this number of events have been processed.
         * @param event_limit Maximum number of events to process.  -1 indicates no limit.
         */
        void setEventLimit(int event_limit=-1) {
            event_limit_ = event_limit;
        }

        /**
         * Get the run mode of the process.
         */
        int getRunMode() {
            return run_mode_;
        }

        /**
         * Get the number of events to skip.
         */
        int getSkipEvents() {
            return skip_events_;
        }

        /** Run the LCIO to ROOT process. */
        void run();

        /** Run the ROOT to Histo process. */
        void runOnRoot();

        /** Run the Histo Analysis process. */
        void runOnHisto();

        /** Request that the processing finish with this event. */ 
        void requestFinish() { event_limit_ = 0; }

    private:

        /** Reader used to parse either binary or EVIO files. */
        //DataRead* data_reader{nullptr}; 

        /** Run mode of the process. */
        int run_mode_{-1};

        /** Number of events to skip. */
        int skip_events_{-1};

        /** Limit on events to process. */
        int event_limit_{-1};

        /** Ordered list of Processors to execute. */
        std::vector<Processor*> sequence_;

        /** List of input files to process.  May be empty if this Process will generate new events. */
        std::vector<std::string> input_files_;

        /** List of output file names.  If empty, no output file will be created. */
        std::vector<std::string> output_files_;

};

#endif
