/**
 * @file HPSEvioProcessor.cxx
 * @brief Post processor of vtx histos
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
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void HPSEvioProcessor::initialize(std::string inFilename, std::string outFilename) {
    std::cout << "HPSEvioProcessor::initialize" << std::endl;
}

bool HPSEvioProcessor::process() {
    
    std::cout << "HPSEvioProcessor::process" << std::endl;
    return true;
}

void HPSEvioProcessor::finalize() { 

    std::cout << "HPSEvioProcessor::finalize" << std::endl;
}

DECLARE_PROCESSOR(HPSEvioProcessor); 
