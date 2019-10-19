/**
 * @file Process.cxx
 * @brief Class which represents the process under execution.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Process.h"
#include "EventFile.h"
#include "HpsEventFile.h"

ClassImp(Process)

Process::Process() {}

//TODO Fix this better

bool Process::processRootFiles() {
    if ((input_files_[0]).find(".root") != std::string::npos)
        return true;

    return false;
}

void Process::runOnRoot() {
    try {
        int n_events_processed = 0;
        HpsEvent event;
        int cfile =0 ;
        for (auto ifile : input_files_) {
            std::cout<<"Processing file"<<ifile<<std::endl;
            HpsEventFile* file(nullptr);
            if (!output_files_.empty()) {
                file = new HpsEventFile(ifile, output_files_[cfile]);
                file->setupEvent(&event);
            }
            for (auto module : sequence_) {
                module->initialize(event.getTree());
            }
            while (file->nextEvent() && (event_limit_ < 0 || (n_events_processed < event_limit_))) {
                if (n_events_processed%1000 == 0)
                    std::cout<<"Event:"<<n_events_processed<<std::endl;

                //In this way if the processing fails (like an event doesn't pass the selection, the other modules aren't run on that event)
                for (auto module : sequence_) {
                    module->process(&event);
                }
                //event.Clear();
                ++n_events_processed;
            }
            //Pass to next file
            ++cfile;
            // Finalize all modules

            //Select the output file for storing the results of the processors.
            file->resetOutputFileDir();
            for (auto module : sequence_) {
                //TODO:Change the finalize method
                module->finalize();
            }
            // TODO Check all these destructors
            if (file) {
                file->close();
                delete file;
                file = nullptr;
            }
        }
    } catch (std::exception& e) {
        std::cerr<<"Error:"<<e.what()<<std::endl;
    }
}

void Process::run() {

    try {

        int n_events_processed = 0;

        if (input_files_.empty()) 
            throw std::runtime_error("Please specify files to process.");

        // Create an object used to manage the input and output files.
        Event event;  

        int cfile = 0; 
        for (auto ifile : input_files_) { 

            std::cout << "---- [ hpstr ][ Process ]: Processing file " 
                << ifile << std::endl;


            //TODO:: Change the order here.

            // Open the output file if an output file path has been specified.
            EventFile* file{nullptr};  
            if (!output_files_.empty()) { 
                file = new EventFile(ifile, output_files_[cfile]);
                file->setupEvent(&event);  
            }

            TTree* tree = event.getTree(); 
            // first, notify everyone that we are starting
            for (auto module : sequence_) {
                module->initialize(tree);
            }

            //In the case of additional output files from the processors this restores the correct ProcessID storage
            file->resetOutputFileDir();

            // Process all events.
            while (file->nextEvent() && (event_limit_ < 0 || (n_events_processed < event_limit_))) {
                if (n_events_processed%1000 == 0)
                    std::cout << "---- [ hpstr ][ Process ]: Event: " << n_events_processed << std::endl;
                event.Clear(); 
                for (auto module : sequence_) {
                    //TODO: actually pass the flag to the filling of the tree
                    if (!module->process(&event))
                        break;
                }
                ++n_events_processed;
            }
            ++cfile; 

            // Finalize all modules. 
            for (auto module : sequence_) { 
                module->finalize(); 
            }

            if (file) {
                file->close(); 
                delete file;
                file = nullptr;
            }

        }

    } catch (std::exception& e) {
        std::cerr << "---- [ hpstr ][ Process ]: Error! " << e.what() << std::endl;
    }
}

void Process::addFileToProcess(const std::string& filename) {
    input_files_.push_back(filename);
}

void Process::addOutputFileName(const std::string& output_filename) {
    output_files_.push_back(output_filename);
}

void Process::addToSequence(Processor* mod) {
    sequence_.push_back(mod);
}

