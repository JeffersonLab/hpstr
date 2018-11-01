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

        // Instantiate the LCIO file reader
        IO::LCReader* lc_reader = IOIMPL::LCFactory::getInstance()->createLCReader(); 
        
        // Create an object used to store all of the event information of the
        // current event.
        EVENT::LCEvent* event{nullptr}; 

        int cfile = 0; 
        for (auto ifile : input_files_) { 
            
            // Open the input file.  If the input file can't be opened, throw
            // an exeception.
            lc_reader->open(ifile); 
        
            std::cout << "---- [ hpstr ][ Process ]: Processing file " 
                      << ifile << std::endl;

            // Open the output file where all histograms will be stored.
            //TFile* ofile = new TFile(output_files_[cfile].c_str(), "RECREATE"); 

            // first, notify everyone that we are starting
            for (auto module : sequence_) {
                module->initialize();
            }


            while ((event = lc_reader->readNextEvent()) != 0) {
                //std::cout << "--- [ hpstr ][ Process ]: Event: " << std::endl;
                for (auto module : sequence_) { 
                    module->process(event); 
                }
            }
            ++cfile; 
         
            for (auto module : sequence_) { 
                module->finalize(); 
            }

            /*if (ofile) {
                ofile->Write();
                delete ofile;
                ofile = nullptr;
            }*/

            // Close the LCIO file that was being processed
            lc_reader->close(); 
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

