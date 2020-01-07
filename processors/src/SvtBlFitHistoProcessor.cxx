#include "SvtBlFitHistoProcessor.h"

SvtBlFitHistoProcessor::SvtBlFitHistoProcessor(const std::string& name, Process& process)
    : Processor(name, process) {
    }

SvtBlFitHistoProcessor::~SvtBlFitHistoProcessor() {
}

void SvtBlFitHistoProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring SvtBlFitHistoProcessor" << std::endl;

}

void SvtBlFitHistoProcessor::initialize(std::string inFilename, std::string outFilename) {

    std::cout << "initializing SvtBlFitHistoProcessor" << std::endl;

}


bool SvtBlFitHistoProcessor::process() { 

    std::cout << "processing SvtBlFitHistoProcessor" << std::endl;

}

void SvtBlFitHistoProcessor::finalize() {

    std::cout << "finalizing SvtBlFitHistoProcessor" << std::endl;

}

DECLARE_PROCESSOR(SvtBlFitHistoProcessor);
