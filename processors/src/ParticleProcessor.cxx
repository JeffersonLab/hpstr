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

void ParticleProcessor::initialize() {
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
            
                TClonesArray* tracks = event->getCollection("Tracks"); 

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
                        if (collections.first.compare(Collections::FINAL_STATE_PARTICLES) == 0) {                    
                            track->setParticle(particle); 
                        } 
                        break; 
                    }
                }
               
            }   

        /*for (auto const &cluster : lc_particle->getClusters()) { 
                
            // Loop through all of the clusters in the HpsEvent and find the one 
            // that matches the cluster associated with the particle
            for (int cluster_n = 0; cluster_n < hps_event->getNumberOfEcalClusters(); ++cluster_n) {

                // Use the cluster energy to find the match
                // TODO: Verify that the cluster enegy is unique enough to find a match
                if (cluster->getEnergy() == hps_event->getEcalCluster(cluster_n)->getEnergy()) {
                    particle->addCluster(hps_event->getEcalCluster(cluster_n)); 
                    break;
                }
            }       
        }*/

            // Only add vertex information if the particle is not a final state particle
            if (collections.first.compare(Collections::FINAL_STATE_PARTICLES) == 0) {                    
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
        // If the particle has daughter particles, add the daughters to the Particle
        //

        // Loop through all of the daughter particles associated with the particle
        /*for (auto const &daughter : lc_particle->getParticles()) { 
                
            // Loop through all of the final state particles in the HpsEvent and
            // find the one that matches the daughters associated with the particles
            for (int d_particle_n = 0; d_particle_n < hps_event->getNumberOfParticles(HpsParticle::FINAL_STATE_PARTICLE); ++d_particle_n) {
                  
                HpsParticle* daughter_particle 
                    = hps_event->getParticle(HpsParticle::FINAL_STATE_PARTICLE, d_particle_n); 

                // Try to find the match between a final state HpsParticle 
                // and ReconstructedParticle daughter.  For now, use the
                // momentum as the matching criterion. 
                // TODO: Verify that the track momentum is always unique in an event.
                if (daughter->getMomentum()[0] == daughter_particle->getMomentum()[0] 
                        && daughter->getMomentum()[1] == daughter_particle->getMomentum()[1]
                        && daughter->getMomentum()[2] == daughter_particle->getMomentum()[2]) {
            
                    particle->addParticle(daughter_particle);
                    
                    if (daughter_particle->getTracks()->GetEntriesFast() != 0) 
                        particle->addTrack((SvtTrack*) daughter_particle->getTracks()->At(0)); 
                    
                    if (daughter_particle->getClusters()->GetEntriesFast() != 0) 
                        particle->addCluster((EcalCluster*) daughter_particle->getClusters()->At(0)); 

                    break; 
                }
            }
        }*/
        }   
    
        // Add the hit collection to the event
        event->addCollection(collections.first, collections.second); 
    }
}

void ParticleProcessor::finalize() { 
}

DECLARE_PROCESSOR(ParticleProcessor); 
