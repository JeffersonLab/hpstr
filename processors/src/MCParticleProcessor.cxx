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

}

void MCParticleProcessor::initialize(TTree* tree) {
    // Create a new TClonesArray collection
    mc_particles_ = new TClonesArray(Collections::MC_PARTICLES, 1000000);  
    // Add branch to tree
    tree->Branch(Collections::MC_PARTICLES,&mc_particles_);

}

bool MCParticleProcessor::process(IEvent* ievent) {

    Event* event = static_cast<Event*> (ievent);

    // Get the collection from the event
    EVENT::LCCollection* lc_particles = event->getLCCollection(Collections::MC_PARTICLES);

    mc_particles_->Clear();

    // Loop through all of the particles in the event
    for (int iparticle = 0; iparticle < lc_particles->getNumberOfElements(); ++iparticle) {

        // Get a particle from the LCEvent
        IMPL::MCParticleImpl* lc_particle
            = static_cast<IMPL::MCParticleImpl*>(lc_particles->getElementAt(iparticle)); 

        // Make an MCParticle to build and add to TClonesArray
        MCParticle* particle = static_cast<MCParticle*>(mc_particles_->ConstructedAt(iparticle));

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

        // Loop through all of the tracks associated with the particle
        // and add references to the MCParticle object.
        /*for (auto const &lc_track : lc_particle->getTracks()) { 

            TClonesArray* tracks = event->getCollection(Collections::GBL_TRACKS); 

            // Loop through all of the tracks in the HpsEvent and find the one
            // that matches the track associated with the particle
            for (int itrack = 0; itrack < tracks->GetEntriesFast(); ++itrack) { 
                Track* track = static_cast<Track*>(tracks->At(itrack)); 

                // Use the track chi^2 to find the match
                // TODO: Verify that the chi^2 is unique enough to find the match
                if (lc_track->getChi2() == track->getChi2()) {

                    // Add a reference to the track 
                    particle->addTrack(track);

                    // If the particle is a final state particle, add a
                    // reference from the corresponding track to the particle
                    if ((collections.first.compare(Collections::FINAL_STATE_PARTICLES) == 0)
                            || (collections.first.compare(Collections::OTHER_ELECTRONS) == 0) ) {                    
                        track->setMCParticle(particle);
                        track->setMomentum(particle->getMomentum()); 
                        track->setCharge(particle->getCharge());  
                    } 
                    break; 
                }
            }

        }*/   

        // Only add vertex information if the particle is not a final state particle
        //if ((collections.first.compare(Collections::FINAL_STATE_PARTICLES) == 0) || 
        //        (collections.first.compare(Collections::OTHER_ELECTRONS) == 0)) {                    
        //    // Set the PDG ID of the particle
        //    particle->setPDG(lc_particle->getParticleIDUsed()->getPDG());    
        //    continue;
        //}

        // Set the vertex position of the particle
        particle->setVertexPosition(lc_particle->getVertex()); 

        // Set the vertex position of the particle
        particle->setEndPoint(lc_particle->getEndpoint()); 

        // If the particle has daughter particles, add the daughters to the
        // MCParticle.
        //

        // Loop through all of the daughter particles associated with the particle
        /*for (auto const &daughter : lc_particle->getParticles()) { 

            // Loop through all of the final state particles in the event 
            // and find the one that matches the daughters associated with 
            // the particles.
            for (int iparticle = 0; 
                    iparticle < fs_particles->GetEntriesFast(); ++iparticle) {

                MCParticle* dparticle = static_cast<MCParticle*>(fs_particles->At(iparticle));   

                // Try to find the match between a final state particle
                // and ReconstructedParticle daughter.  For now, use the
                // momentum as the matching criterion. 
                // TODO: Verify that the track momentum is always unique in an event.
                if ((dparticle->getMomentum()[0] == lc_particle->getMomentum()[0])
                        && (dparticle->getMomentum()[1] == lc_particle->getMomentum()[1])
                        && (dparticle->getMomentum()[2] == lc_particle->getMomentum()[2])) {

                    particle->addDaughter(dparticle);

                    if (dparticle->getTracks()->GetEntriesFast() != 0) 
                        particle->addTrack(dparticle->getTracks()->At(0)); 

                    if (dparticle->getClusters()->GetEntriesFast() != 0) 
                        particle->addCluster(dparticle->getClusters()->At(0)); 

                    break; 
                }
            }
        }*/
    }   

    return true;
}

void MCParticleProcessor::finalize() { 
}

DECLARE_PROCESSOR(MCParticleProcessor); 
