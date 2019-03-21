/**
 * @file ParticleProcessor.cxx
 * @brief Processor used to translate LCIO ReconstructedParticles to DST 
 *        Particle objects.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "ParticleProcessor.h"

ParticleProcessor::ParticleProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
}

ParticleProcessor::~ParticleProcessor() { 
}

void ParticleProcessor::configure(const ParameterSet& parameters) {
    
    std::vector<std::string> collection_names = parameters.getVString("Collections"); 
    for (auto const &collection_name : collection_names) { 
        std::cout << "[ ParticleProcessor ]: Processing Collection: "
                  << collection_name << std::endl;

        // Create a new TClonesArray collection
        collections_[collection_name] = new TClonesArray("Particle", 1000000);  
    }

}

void ParticleProcessor::initialize(TTree* tree) {
}

void ParticleProcessor::process(Event* event) {

    for (auto& collections : collections_) { 
       
        // Get the collection from the event
        EVENT::LCCollection* lc_particles = event->getLCCollection(collections.first);

        // Loop through all of the particles in the event
        for (int iparticle = 0; iparticle < lc_particles->getNumberOfElements(); ++iparticle) {

            // Get a particle from the LCEvent
            EVENT::ReconstructedParticle* lc_particle 
                = static_cast<EVENT::ReconstructedParticle*>(lc_particles->getElementAt(iparticle)); 

            // Add a particle to the event
            Particle* particle = static_cast<Particle*>(collections.second->ConstructedAt(iparticle));

            // Set the charge of the HpsParticle    
            particle->setCharge(lc_particle->getCharge());

            // Set the HpsParticle type
            particle->setType(lc_particle->getType());  
        
            // Set the energy of the HpsParticle
            particle->setEnergy(lc_particle->getEnergy());

            // Set the momentum of the HpsParticle
            particle->setMomentum(lc_particle->getMomentum()); 

            // Set the mass of the HpsParticle
            particle->setMass(lc_particle->getMass());

            // Set the goodness of PID for the HpsParticle
            particle->setGoodnessOfPID(lc_particle->getGoodnessOfPID()); 

            // Loop through all of the tracks associated with the particle
            // and add references to the Particle object.
            for (auto const &lc_track : lc_particle->getTracks()) { 
            
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
                            track->setParticle(particle);
                            track->setMomentum(particle->getMomentum()); 
                            track->setCharge(particle->getCharge());  
                        } 
                        break; 
                    }
                }
               
            }   
            
            for (auto const &lc_cluster : lc_particle->getClusters()) { 
              
                std::string coll_name = Collections::ECAL_TIME_CORR_HITS;
                if (!event->exists(coll_name))  coll_name = Collections::ECAL_HITS;

                // Get the collection of ECal hits from the event.
                TClonesArray* clusters = event->getCollection(coll_name); 

                // Loop through all of the clusters in the event and find the one 
                // that matches the cluster associated with the particle
                for (int icluster = 0; icluster < clusters->GetEntriesFast(); ++icluster) {

                    CalCluster* cluster = static_cast<CalCluster*>(clusters->At(icluster)); 
                    // Use the cluster energy to find the match
                    // TODO: Verify that the cluster enegy is unique enough to find a match
                    if (lc_cluster->getEnergy() == cluster->getEnergy()) {
                        particle->addCluster(cluster);
                        break;
                    }
                }          
            }

            // Only add vertex information if the particle is not a final state particle
            if ((collections.first.compare(Collections::FINAL_STATE_PARTICLES) == 0) || 
                    (collections.first.compare(Collections::OTHER_ELECTRONS) == 0)) {                    
                // Set the PDG ID of the particle
                particle->setPDG(lc_particle->getParticleIDUsed()->getPDG());    
                continue;
            }
        
            // Set the vertex position of the particle
            EVENT::Vertex* vtx = static_cast<EVENT::Vertex*>(lc_particle->getStartVertex()); 
            particle->setVertexPosition(vtx->getPosition()); 

            // Set the vertex chi2
            particle->setVertexFitChi2(vtx->getChi2()); 

            //
            // If the particle has daughter particles, add the daughters to the
            // Particle.
            //

            // Get the collection of final state particles from the event.  If 
            // it doesn't exist, continue on to the next collection. 
            if (!event->exists(Collections::FINAL_STATE_PARTICLES)) continue; 

            // Get the collection of final state particles from the event.
            TClonesArray* fs_particles 
                = event->getCollection(Collections::FINAL_STATE_PARTICLES); 

            // Loop through all of the daughter particles associated with the particle
            for (auto const &daughter : lc_particle->getParticles()) { 
                
                // Loop through all of the final state particles in the event 
                // and find the one that matches the daughters associated with 
                // the particles.
                for (int iparticle = 0; 
                        iparticle < fs_particles->GetEntriesFast(); ++iparticle) {
                
                Particle* dparticle = static_cast<Particle*>(fs_particles->At(iparticle));   

                    // Try to find the match between a final state particle
                    // and ReconstructedParticle daughter.  For now, use the
                    // momentum as the matching criterion. 
                    // TODO: Verify that the track momentum is always unique in an event.
                    if ((dparticle->getMomentum()[0] == lc_particle->getMomentum()[0])
                            && (dparticle->getMomentum()[1] == lc_particle->getMomentum()[1])
                            && (dparticle->getMomentum()[2] == lc_particle->getMomentum()[2])) {
            
                        particle->addParticle(dparticle);
                    
                        if (dparticle->getTracks()->GetEntriesFast() != 0) 
                            particle->addTrack(dparticle->getTracks()->At(0)); 
                    
                        if (dparticle->getClusters()->GetEntriesFast() != 0) 
                            particle->addCluster(dparticle->getClusters()->At(0)); 

                        break; 
                    }
                }
            }
        }   
    
        // Add the hit collection to the event
        event->addCollection(collections.first, collections.second); 
    }
}

void ParticleProcessor::finalize() { 
}

DECLARE_PROCESSOR(ParticleProcessor); 
