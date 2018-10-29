/**
 * @file Process.cxx
 * @brief Class which represents the process under execution.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Process.h"

Process::Process() {}

void Process::run() {

    try {
    
        int n_events_processed = 0;
        
        if (input_files_.empty()) 
            throw std::runtime_error("Please specify files to process.");

        //data_reader = new DataRead();
        //TrackerEvent event;  

        int cfile = 0; 
        for (auto ifile : input_files_) { 
            
            // Open the input file.  If the input file can't be opened, exit
            // the application.
            //if (!data_reader->open(ifile.c_str(), false)) 
            //  throw std::runtime_error("Error! File " + ifile + " cannot be opened.");   
        
            std::cout << "---- [ svt-qa ][ Process ]: Processing file " 
                      << ifile << std::endl;

            // Open the output file where all histograms will be stored.
            TFile* ofile = new TFile(output_files_[cfile].c_str(), "RECREATE"); 

            // first, notify everyone that we are starting
            for (auto module : sequence_) {
                module->initialize();
            }


            //while (data_reader->next(&event)) {
            //    for (auto module : sequence_) { 
            //        module->process(&event); 
            //    }
            //}
            ++cfile; 
         
            for (auto module : sequence_) { 
                module->finalize(); 
            }

            if (ofile) {
                ofile->Write();
                delete ofile;
                ofile = nullptr;
            }
        }

    } catch (std::exception& e) {
        std::cerr << "---- [ svt-qa ][ Process ]: Error! " << e.what() << std::endl;
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

