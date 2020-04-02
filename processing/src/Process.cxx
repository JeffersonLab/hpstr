/**
 * @file Process.cxx
 * @brief Class which represents the process under execution.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 * @author PF, SLAC National Accelerator Laboratory
 */

#include "Process.h"
#include "EventFile.h"
#include "HpsEventFile.h"
#include "TH1.h"

Process::Process() {}

//TODO Fix this better

void Process::runOnHisto() {
    try {
        int cfile = 0;
        for (auto ifile : input_files_) {
            std::cout << "Processing file " << ifile << std::endl;

            for (auto module : sequence_) {
                module->initialize(ifile, output_files_[cfile]);
                module->process();
                module->finalize();
            }
            //Pass to next file
            ++cfile;

        } //ifile
    } catch (std::exception& e) {
        std::cerr<<"Error:"<<e.what()<<std::endl;
    }
} //Process::runOnHisto

void Process::runOnRoot() {
    try {
        int n_events_processed = 0;
        HpsEvent event;
        TH1D * event_h = new TH1D("event_h","Number of Events Processed;;Events", 21, -10.5, 10.5);
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
                module->setFile(file->getOutputFile());
            }
            while (file->nextEvent() && (event_limit_ < 0 || (n_events_processed < event_limit_))) {
                if (n_events_processed%1000 == 0)
                    std::cout<<"Event:"<<n_events_processed<<std::endl;

                //In this way if the processing fails (like an event doesn't pass the selection, the other modules aren't run on that event)
                for (auto module : sequence_) {
                    module->process(&event);
                }
                //event.Clear();
                event_h->Fill(0.0);
                ++n_events_processed;
            }
            //Pass to next file
            ++cfile;
            // Finalize all modules

            //Select the output file for storing the results of the processors.
            file->resetOutputFileDir();
            event_h->Write();
            for (auto module : sequence_) {
                //TODO:Change the finalize method
                module->finalize();
            }
            // TODO Check all these destructors
            if (file) {
                file->close();
                delete file;
                file = nullptr;
                delete event_h;
                event_h = nullptr;
            }
        }
    } catch (std::exception& e) {
        std::cerr<<"Error:"<<e.what()<<std::endl;
    }
}

void Process::run() {

    try {

        int n_events_processed = 0;
        TH1D * event_h = new TH1D("event_h","Number of Events Processed;;Events", 21, -10.5, 10.5);

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

            TTree* tree = new TTree("HPS_Event","HPS event tree");
            event.setTree(tree); 
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
                bool passEvent = true;
                
                for (auto module : sequence_) {
                    passEvent = passEvent && module->process(&event);
                    //if (!module->process(&event))
                    if (!passEvent)
                        break;
                }
                ++n_events_processed;
                event_h->Fill(0.0);
                if (passEvent) {
                    file->FillEvent();
                }
            }
            ++cfile; 

            //Prepare to write to file
            file->resetOutputFileDir();
            event_h->Write();
            // Finalize all modules. 
            for (auto module : sequence_) { 
                module->finalize(); 
            }

            if (file) {
                file->close(); 
                delete file;
                file = nullptr;
                delete event_h;
                event_h = nullptr;
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

