/**
 * @file HPSEvioProcessor.cxx
 * @author PF, SLAC National Accelerator Laboratory
 */

#include "HPSEvioProcessor.h"

HPSEvioProcessor::HPSEvioProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
    }

HPSEvioProcessor::~HPSEvioProcessor() { 
}

void HPSEvioProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring HPSEvioProcessor" << std::endl;
    try
    {
        debug_          = parameters.getInteger("debug");
        trigFilename_   = parameters.getString("trigConf");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void HPSEvioProcessor::initialize(std::string inFilename, std::string outFilename) {
    std::cout << "HPSEvioProcessor::initialize" << std::endl;
    etool = new HPSEvioReader();
    etool->Open(inFilename.c_str());
    etool->SVT->fSaveHeaders = true;



    if(trigFilename_.size()>2){
        etool->TrigConf->Parse_trigger_file(trigFilename_); // If requested, parse the Trigger config file supplied.
        etool->ECAL->Config();
        cout << "Parsed trigger file: " << trigFilename_ << endl;
    }

    //DEBUGGING  
    if(debug_==0){
        etool->fDebug = 0b000000;
    }else if(debug_==1){
        etool->fDebug = 0b000001;
    }else if(debug_==2){
        etool->fDebug = 0b000011;
    } else if(debug_ == 3){
        etool->fDebug = 0b000111;
    } else{
        etool->fDebug = 0xFF;
    }


    bool found = false;
    while( !found &&   etool->Next()== S_SUCCESS){
        if( (etool->this_tag & 128) == 128){
            found = true;
            run_number_ = etool->GetRunNumber();
            trigtime_start_ = etool->GetTrigTime();
            break;
        }
    }
    if( found == false) std::cout << "WARNING -- Not able to find a bank with a runnumber! \n";
    etool->Close();

    if(etool->SVT){
        if(run_number_ < 8000){ // This is 2015 or 2016 data.
            etool->Set2016Data();
        }else{
            etool->Set2019Data();
        }
    }else{
        cout << "NO SVT initialized \n";
    }

    outF_ = new TFile(outFilename.c_str(),"RECREATE");
    etool->fAutoAdd = false;

    cout << "Debug set to " << etool->fDebug << " Auto add = " << etool->fAutoAdd << endl;

    etool->PrintBank(5);
}

bool HPSEvioProcessor::process() {

    std::cout << "HPSEvioProcessor::process" << std::endl;
    return true;
}

void HPSEvioProcessor::finalize() { 

    std::cout << "HPSEvioProcessor::finalize" << std::endl;
}

DECLARE_PROCESSOR(HPSEvioProcessor); 
