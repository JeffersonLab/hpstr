/** 
 * @file FinalStateParticleProcessor.h
 * @brief Class used to convert LCIO Vertex collections into ROOT collections.
 * @author Cameron Bravo, SLAC
 */
#include "FinalStateParticleProcessor.h" 
#include "utilities.h"

FinalStateParticleProcessor::FinalStateParticleProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
    }

FinalStateParticleProcessor::~FinalStateParticleProcessor() { 
}

void FinalStateParticleProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring FinalStateParticleProcessor" << std::endl;
    try
    {
        debug_             = parameters.getInteger("debug", debug_);
        fspCollLcio_       = parameters.getString("fspCollLcio", fspCollLcio_);
        fspCollRoot_       = parameters.getString("fspCollRoot", fspCollRoot_);
        kinkRelCollLcio_   = parameters.getString("kinkRelCollLcio", kinkRelCollLcio_);
        trkRelCollLcio_    = parameters.getString("trkRelCollLcio", trkRelCollLcio_);
        
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void FinalStateParticleProcessor::initialize(TTree* tree) {
    // Add branches to tree
    tree->Branch(fspCollRoot_.c_str(),  &fsps_);
}

bool FinalStateParticleProcessor::process(IEvent* ievent) {

    if (debug_ > 0) std::cout << "FinalStateParticleProcessor: Clear output vector" << std::endl;
    for(int i = 0; i < fsps_.size(); i++) delete fsps_.at(i);
    fsps_.clear();

    Event* event = static_cast<Event*> (ievent);

    // Get the collection of vertices from the LCIO event. If no such collection 
    // exist, a DataNotAvailableException is thrown
    if (debug_ > 0) std::cout << "FinalStateParticleProcessor: Get LCIO Collection " << fspCollLcio_ << std::endl;
    EVENT::LCCollection* lc_fsps= nullptr;
    try
    {
        lc_fsps = event->getLCCollection(fspCollLcio_.c_str()); 
    }
    catch (EVENT::DataNotAvailableException e) 
    {
        std::cout << e.what() << std::endl;
        return false;
    }

    // Get the collection of LCRelations between GBL tracks and kink data and track data variables.
    EVENT::LCCollection* gbl_kink_data{nullptr};
    EVENT::LCCollection* track_data{nullptr};
        
    try
    {
        if (!kinkRelCollLcio_.empty())
            gbl_kink_data = static_cast<EVENT::LCCollection*>(event->getLCCollection(kinkRelCollLcio_.c_str()));
        if (!trkRelCollLcio_.empty())
            track_data = static_cast<EVENT::LCCollection*>(event->getLCCollection(trkRelCollLcio_.c_str()));
    }
    catch (EVENT::DataNotAvailableException e)
    {
        std::cout << e.what() << std::endl;
        if (!gbl_kink_data)
            std::cout<<"Failed retrieving " << kinkRelCollLcio_ <<std::endl;
        if (!track_data)
            std::cout<<"Failed retrieving " << trkRelCollLcio_ <<std::endl;
        
    }
    
    
    if (debug_ > 0) std::cout << "FinalStateParticleProcessor: Converting"<< std::endl;
    for (int ifsp = 0 ; ifsp < lc_fsps->getNumberOfElements(); ++ifsp) 
    {
        if (debug_ > 0) std::cout << "FinalStateParticleProcessor: Converting FinalStateParticle " << ifsp << std::endl;
        EVENT::ReconstructedParticle* lc_fsp{nullptr};
        lc_fsp = static_cast<EVENT::ReconstructedParticle*>(lc_fsps->getElementAt(ifsp));
        if (debug_ > 0) std::cout << "FinalStateParticleProcessor: Build Particle" << std::endl;
        Particle * fsp = utils::buildParticle(lc_fsp, gbl_kink_data, track_data);
        if (debug_ > 0) std::cout << "FinalStateParticleProcessor: Add Particle" << std::endl;
        fsps_.push_back(fsp);
    }

    if (debug_ > 0) std::cout << "FinalStateParticleProcessor: End process" << std::endl;
    return true;
}

void FinalStateParticleProcessor::finalize() { 
}

DECLARE_PROCESSOR(FinalStateParticleProcessor); 
