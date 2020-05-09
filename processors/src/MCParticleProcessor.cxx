/**
 * @file MCParticleProcessor.cxx
 * @brief Processor used to translate LCIO ReconstructedParticles to DST 
 *        MCParticle objects.
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */

#include "MCParticleProcessor.h"

MCParticleProcessor::MCParticleProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
    }

MCParticleProcessor::~MCParticleProcessor() { 
}

void MCParticleProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring MCParticleProcessor" << std::endl;
    try
    {
        debug_             = parameters.getInteger("debug", debug_ );
        mcPartCollLcio_    = parameters.getString("mcPartCollLcio", mcPartCollLcio_);
        mcPartCollRoot_    = parameters.getString("mcPartCollRoot", mcPartCollRoot_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }


}

void MCParticleProcessor::initialize(TTree* tree) {
    
    // Add branch to tree
    tree->Branch(mcPartCollRoot_.c_str(),&mc_particles_);

}

bool MCParticleProcessor::process(IEvent* ievent) {

    // Get the simulated tracker hit count
    auto event{static_cast<Event*> (ievent)};

    // Get the collection of MC particles from the LCIO event.
    EVENT::LCCollection* lc_particles{nullptr};
    try {
        lc_particles = event->getLCCollection(mcPartCollLcio_.c_str());
    } catch (EVENT::DataNotAvailableException e) {
        std::cout << e.what() << std::endl;
    }

    // Clear any previously stored data
    clear(); 
    
    // Get the number of MC particles
    auto particle_count{lc_particles->getNumberOfElements()};

    // Loop through all of the particles in the event
    for (int iparticle{0}; iparticle < particle_count; ++iparticle) { 

        // Get a particle from the LCEvent
        auto lc_particle{static_cast<IMPL::MCParticleImpl*>(lc_particles->getElementAt(iparticle))}; 
        
        // Instantiate an instance of MC Particle
        auto particle{new MCParticle()}; 

        // Set the charge of the HpsMCParticle    
        particle->setCharge(lc_particle->getCharge());

        // Set the HpsMCParticle type
        particle->setTime(lc_particle->getTime());  

        // Set the energy of the HpsMCParticle
        particle->setEnergy(lc_particle->getEnergy());

        // Set the momentum of the HpsMCParticle
        particle->setMomentum(lc_particle->getMomentum()); 

        // Set the mass of the HpsMCParticle
        particle->setMass(lc_particle->getMass());

        // Set the PDG of the particle
        particle->setPDG(lc_particle->getPDG());    

        // Set the LCIO id of the particle
        particle->setID(lc_particle->id());    

        // Set the generator status of the particle
        particle->setGenStatus(lc_particle->getGeneratorStatus());    

        // Set the generator status of the particle
        particle->setSimStatus(lc_particle->getSimulatorStatus());    

        // Set the PDG of the particle
        std::vector<EVENT::MCParticle*> parentVec = lc_particle->getParents();
        if(parentVec.size() > 0) particle->setMomPDG(parentVec.at(0)->getPDG());

        mc_particles_[lc_particle->id()] = particle; 
    
    }   

    return true;
}

void MCParticleProcessor::finalize() { 
}

void MCParticleProcessor::clear() {
   
    for ( auto element : mc_particles_ ) { 
        delete element.second; 
    }
    mc_particles_.clear(); 

}

DECLARE_PROCESSOR(MCParticleProcessor); 
