/**
 * @file StdhepMCParticleProcessor.cxx
 * @brief Processor used to translate StdHep MCParticles to ROOT
 *      MCParticle objects
 */

#include "StdhepMCParticleProcessor.h" 
#include "utilities.h"

StdhepMCParticleProcessor::StdhepMCParticleProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
    }

StdhepMCParticleProcessor::~StdhepMCParticleProcessor() { 
}

void StdhepMCParticleProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring StdhepMCParticleProcessor" << std::endl;
    try{
        mcPartCollStdhep_ = parameters.getString("mcPartCollStdhep", mcPartCollStdhep_);
        mcPartCollRoot_ = parameters.getString("mcPartCollRoot", mcPartCollRoot_);
        maxEvent_ = parameters.getInteger("maxEvent",maxEvent_);
        skipEvent_ = parameters.getInteger("skipEvent",skipEvent_);   
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void StdhepMCParticleProcessor::initialize(std::string inFilename, std::string outFilename) {
    // Init Files
    std::cout << "[StdhepMCParticleProcessor] initialize" << std::endl;
    inFilename_ = inFilename;
    outF_ = new TFile(outFilename.c_str(),"RECREATE");

    std::cout << "Convert " << inFilename_ << " to ROOT " << outF_->GetName() << std::endl;

    //TTree to store stdhep as root
    tree_ = new TTree("HPS_Event","conversion"); 
    tree_->Branch(mcPartCollRoot_.c_str(),&mc_particles_);
}

bool StdhepMCParticleProcessor::process() {
    std::cout << "[StdhepMCParticleProcessor] Starting process()" << std::endl;
       
    std::cout << "opening file : " << inFilename_ << std::endl;
    UTIL::LCStdHepRdr rdr(inFilename_.c_str());
    rdr.printHeader();
    std::stringstream description;

    description << " file generated with LCIO stdhepjob from "  << inFilename_;

    int count = skipEvent_;

    try {
     
        while( maxEvent_ < 0  || count < maxEvent_ ){

           if (mc_particles_.size() > 0){
                for (std::vector<MCParticle*>::iterator it = mc_particles_.begin(); it != mc_particles_.end(); ++it){
                    delete *it;
                }
                mc_particles_.clear();
            }
             
            std::unique_ptr<IMPL::LCEventImpl> evt( new IMPL::LCEventImpl() ) ;
            evt->setRunNumber(0) ;
            evt->setEventNumber(count) ;
             
            // read the next stdhep event and add an MCParticle collection to the event
            rdr.updateNextEvent(evt.get(),mcPartCollStdhep_.c_str()) ;

            //Get collection from event
            EVENT::LCCollection* lc_particles{nullptr};
            try
            {
                lc_particles = evt->getCollection(mcPartCollStdhep_.c_str());
                //Loop through all particles in event
                for (int iparticle = 0; iparticle < lc_particles->getNumberOfElements(); ++iparticle){
                    //Get particle from LCEvent
                    IMPL::MCParticleImpl* lc_particle = static_cast<IMPL::MCParticleImpl*>(lc_particles->getElementAt(iparticle));

                    //Make MCParticle to build
                    MCParticle* particle = new MCParticle();

                    //Set charge of HpsMCParticle
                    particle->setCharge(lc_particle->getCharge());

                    // Set the HpsMCParticle type
                    particle->setTime(lc_particle->getTime());

                    // Set the energy of the HpsMCParticle
                    particle->setEnergy(lc_particle->getEnergy());

                    // Set the momentum of the HpsMCParticle
                    particle->setMomentum(lc_particle->getMomentum());

                    // Set the momentum of HpsMCParticle at Endpoint
                    particle->setEndpointMomentum(lc_particle->getMomentumAtEndpoint());

                    // Set the mass of the HpsMCParticle
                    particle->setMass(lc_particle->getMass());

                    // Set the PDG of the particle
                    particle->setPDG(lc_particle->getPDG());

                    // Set the LCIO id of the particle
                    particle->setID(lc_particle->id());

                    // Set the PDG of the particle
                    std::vector<EVENT::MCParticle*> parentVec = lc_particle->getParents();
                    if(parentVec.size() > 0) particle->setMomPDG(parentVec.at(0)->getPDG());

                    // Set the generator status of the particle
                    particle->setGenStatus(lc_particle->getGeneratorStatus());

                    // Set the generator status of the particle
                    particle->setSimStatus(lc_particle->getSimulatorStatus());
                    
                    // Set the vertex position of the particle
                    particle->setVertexPosition(lc_particle->getVertex());

                    // Set the vertex position of the particle
                    particle->setEndPoint(lc_particle->getEndpoint());

                    mc_particles_.push_back(particle);
                }

                tree_->Fill();
            }
            catch (EVENT::DataNotAvailableException e)
            {
                std::cout << e.what() << std::endl;
            }
             
            ++count ;
             
        } // evt loop
    }

    catch( IO::EndOfDataException& e ) {  
    }

    std::cout << "  converted " << count <<  std::endl ;

    std::cout << "==================================================== " 
         << std::endl << std::endl ;

    return true;
}

void StdhepMCParticleProcessor::finalize() { 
    outF_->cd();
    outF_->Write();
    outF_->Close();
}

DECLARE_PROCESSOR(StdhepMCParticleProcessor); 
