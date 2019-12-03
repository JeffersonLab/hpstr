#include "VertexProcessor.h" 
#include "utilities.h"

VertexProcessor::VertexProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
}

VertexProcessor::~VertexProcessor() { 
}

void VertexProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring VertexProcessor" << std::endl;
    try
    {
        debug_         = parameters.getInteger("debug");
        vtxCollLcio_   = parameters.getString("vtxCollLcio");
        vtxCollRoot_   = parameters.getString("vtxCollRoot");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void VertexProcessor::initialize(TTree* tree) {
    // Add branches to tree
    tree->Branch(vtxCollRoot_.c_str(), &vtxs_);
}

bool VertexProcessor::process(IEvent* ievent) {

    if (debug_ > 0) std::cout << "VertexProcessor: Clear output vector" << std::endl;
    for(int i = 0; i < vtxs_.size(); i++) delete vtxs_.at(i);
    vtxs_.clear();

    Event* event = static_cast<Event*> (ievent);

    // Get the collection of 3D hits from the LCIO event. If no such collection 
    // exist, a DataNotAvailableException is thrown
    if (debug_ > 0) std::cout << "VertexProcessor: Get LCIO Collection " << vtxCollLcio_ << std::endl;
    EVENT::LCCollection* vtxsLcio = nullptr;
    try
    {
        vtxsLcio = event->getLCCollection(vtxCollLcio_.c_str()); 
    }
    catch (EVENT::DataNotAvailableException e) 
    {
        std::cout << e.what() << std::endl;
        return false;
    }

    if (debug_ > 0) std::cout << "VertexProcessor: Converting Verteces" << std::endl;
    for (int ivtx = 0 ; ivtx < vtxsLcio->getNumberOfElements(); ++ivtx) {
    if (debug_ > 0) std::cout << "VertexProcessor: Converting Vertex " << ivtx << std::endl;
        Vertex* vtx = utils::buildVertex(static_cast<EVENT::Vertex*>(vtxsLcio->getElementAt(ivtx)));
        vtxs_.push_back(vtx);
    }

    return true;
}

void VertexProcessor::finalize() { 
}

DECLARE_PROCESSOR(VertexProcessor); 
