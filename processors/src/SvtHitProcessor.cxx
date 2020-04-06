/**
 * @file SvtHitProcessor.cxx
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SvtHitProcessor.h" 

#include <iostream> 

#include "Event.h"
#include "FlatTupleMaker.h"

#include "EVENT/LCCollection.h"
#include "EVENT/MCParticle.h"
#include "EVENT/SimTrackerHit.h"
#include "EVENT/TrackerHit.h"
#include "EVENT/TrackerRawData.h"
#include "UTIL/LCRelationNavigator.h"

SvtHitProcessor::SvtHitProcessor(const std::string& name, Process& process) 
    : Processor(name, process) { 
}

SvtHitProcessor::~SvtHitProcessor() { 
}

void SvtHitProcessor::initialize(TTree* tree) {
    
    ntuple_ = std::make_shared<FlatTupleMaker>("SvtHits");
  
    // Raw hits 
    ntuple_->addVariable( "raw_hit_count"  ); 
    ntuple_->addVector(   "raw_hit_layer"  ); 
    ntuple_->addVector(   "raw_hit_strip"  );
    ntuple_->addVector(   "raw_hit_module" );
    ntuple_->addVector(   "raw_hit_adc0"   );  
    ntuple_->addVector(   "raw_hit_adc1"   );  
    ntuple_->addVector(   "raw_hit_adc2"   );  
    ntuple_->addVector(   "raw_hit_adc3"   );  
    ntuple_->addVector(   "raw_hit_adc4"   );  
    ntuple_->addVector(   "raw_hit_adc5"   ); 
    ntuple_->addVector(   "raw_hit_is_top" );  
    ntuple_->addVector(   "raw_hit_is_bot" ); 
    ntuple_->addVector(   "raw_hit_is_ele_side");  
    ntuple_->addVector(   "raw_hit_is_pos_side");  
    
    // 1D Strip hits
    ntuple_->addVariable( "strip_cluster_count"  ); 
    ntuple_->addVector(   "strip_cluster_amp"    ); 
    ntuple_->addVector(   "strip_cluster_layer"  ); 
    ntuple_->addVector(   "strip_cluster_module" );
    ntuple_->addVector(   "strip_cluster_is_top" );  
    ntuple_->addVector(   "strip_cluster_is_bot" );
    ntuple_->addVector(   "strip_cluster_is_ele_side" );  
    ntuple_->addVector(   "strip_cluster_is_pos_side" );
    ntuple_->addVector(   "strip_cluster_size"   );
    ntuple_->addVector(   "strip_cluster_x" ); 
    ntuple_->addVector(   "strip_cluster_y" ); 
    ntuple_->addVector(   "strip_cluster_z" ); 

    // Simulated tracker hits
    ntuple_->addVariable( "sim_hit_count" ); 
    ntuple_->addVector(   "sim_hit_layer" ); 
    ntuple_->addVector(   "sim_hit_module" );
    ntuple_->addVector(   "sim_hit_is_top" );  
    ntuple_->addVector(   "sim_hit_is_bot" ); 
    ntuple_->addVector(   "sim_hit_is_ele_side");  
    ntuple_->addVector(   "sim_hit_is_pos_side");  
    ntuple_->addVector(   "sim_hit_raw_strip" ); 
    ntuple_->addVector(   "sim_hit_strip_res_x" );
    ntuple_->addVector(   "sim_hit_strip_res_y" );
    ntuple_->addVector(   "sim_hit_strip_res_z" );
    ntuple_->addVector(   "sim_hit_strip_size" ); 
    ntuple_->addVector(   "sim_hit_strip_res_xerr" );
    ntuple_->addVector(   "sim_hit_strip_res_yerr" );
    ntuple_->addVector(   "sim_hit_strip_res_zerr" );
    ntuple_->addVector(   "sim_hit_x" );  
    ntuple_->addVector(   "sim_hit_y" );  
    ntuple_->addVector(   "sim_hit_z" );  
}

bool SvtHitProcessor::process(IEvent* ievent) {

    // Cast the event to an LCEvent
    auto event{static_cast<Event*>(ievent)};

    // Check for the existence of RawTrackerHits.  If they don't exists, 
    // stop processing the event.
    if (!event->hasLCCollection("SVTRawTrackerHits")) return false;

    // Get the collection of raw tracker hits from the event.
    auto raw_hits{event->getLCCollection("SVTRawTrackerHits")}; 

    auto raw_hit_count{raw_hits->getNumberOfElements()}; 
    ntuple_->setVariableValue("raw_hit_count", raw_hit_count); 

    for (int ihit{0}; ihit< raw_hit_count; ++ihit) {
         
        auto raw_hit{static_cast<EVENT::TrackerRawData*>(raw_hits->getElementAt(ihit))};
        
        // Decode the cell ID
        auto fields{decodeID(raw_hit)}; 

        ntuple_->addToVector("raw_hit_layer", std::get< 0 >(fields)); 
        ntuple_->addToVector("raw_hit_strip", std::get< 2 >(fields)); 
        ntuple_->addToVector("raw_hit_module", std::get< 1 >(fields));
        ntuple_->addToVector("raw_hit_is_top", isTopLayer(std::get< 1 >(fields))); 
        ntuple_->addToVector("raw_hit_is_bot", isBottomLayer(std::get< 1 >(fields)));
        ntuple_->addToVector("raw_hit_is_ele_side",  isElectronSide(std::get< 1 >(fields))); 
        ntuple_->addToVector("raw_hit_is_pos_side",  isPositronSide(std::get< 1 >(fields))); 
        
        std::vector< short > adc_values = raw_hit->getADCValues();
        for (int iadc{0}; iadc < adc_values.size(); ++iadc) {
            ntuple_->addToVector("raw_hit_adc" + std::to_string(iadc), adc_values[iadc]); 
        }
    }
    std::map < EVENT::TrackerRawData*, EVENT::TrackerHit* > hit_map; 
    if (event->hasLCCollection("StripClusterer_SiTrackerHitStrip1D")) { 
        
        auto strip_hits{event->getLCCollection("StripClusterer_SiTrackerHitStrip1D")}; 

        auto strip_hit_count{strip_hits->getNumberOfElements()};
        ntuple_->setVariableValue("strip_cluster_count", strip_hit_count); 

        for (int ihit{0}; ihit < strip_hit_count; ++ihit) { 
            
            auto hit{static_cast<EVENT::TrackerHit*>(strip_hits->getElementAt(ihit))}; 

            auto position{hit->getPosition()}; 
            ntuple_->addToVector("strip_cluster_x", position[0]); 
            ntuple_->addToVector("strip_cluster_y", position[1]); 
            ntuple_->addToVector("strip_cluster_z", position[2]); 
        
            auto strip_raw_hits{hit->getRawHits()}; 
            ntuple_->addToVector("strip_cluster_size", strip_raw_hits.size());
            
            // Decode the cell ID
            auto fields{decodeID(static_cast<EVENT::TrackerRawData*>(strip_raw_hits[0]))}; 
            
            ntuple_->addToVector("strip_cluster_layer", std::get<0>(fields));  
            ntuple_->addToVector("strip_cluster_module", std::get<1>(fields));  
            ntuple_->addToVector("strip_cluster_is_top", isTopLayer(std::get< 1 >(fields))); 
            ntuple_->addToVector("strip_cluster_is_bot", isBottomLayer(std::get< 1 >(fields)));
            ntuple_->addToVector("strip_cluster_is_ele_side",  isElectronSide(std::get< 1 >(fields))); 
            ntuple_->addToVector("strip_cluster_is_pos_side",  isPositronSide(std::get< 1 >(fields))); 

            for (const auto& strip_raw_hit : strip_raw_hits) 
                hit_map[static_cast<EVENT::TrackerRawData*>(strip_raw_hit)] = hit; 
        }
    }

    // Check for the existence of simulated tracker hits.  If they don't 
    // exists, stop processing the event. 
    if (!event->hasLCCollection("TrackerHits")) {
        ntuple_->fill(); 
        return false;
    }

    // Get the collection of simulated tracker hits from the event. 
    auto sim_hits{event->getLCCollection("TrackerHits")}; 

    // Instantiate the tracker hit ID decoder
    UTIL::BitField64 decoder("system:0:6,barrel:6:3,layer:9:4,module:13:12,sensor:25:1,side:32:-2,strip:34:12");

    // Check if the LCRelation between sim hits and raw hits exists.  If it doesn't, stop
    // processing the rest of the event. 
    //if (!event->hasLCCollection("SVTTrueHitRelations")) return false; 

    // Get the relations between a sim hit and raw hits
    auto raw_sim_relation{event->getLCCollection("SVTTrueHitRelations")};

    // Instantiate an LCRelations navigator which allows faster access to the 
    // raw tracker hits associated with a given sim tracker hit.
    auto raw_sim_nav{new UTIL::LCRelationNavigator(raw_sim_relation)};

    auto sim_hit_count = sim_hits->getNumberOfElements(); 
    ntuple_->setVariableValue("sim_hit_count", sim_hit_count);

    for (int ihit{0}; ihit < sim_hit_count; ++ihit) { 
        
        auto sim_hit{static_cast<EVENT::SimTrackerHit*>(sim_hits->getElementAt(ihit))};

        ntuple_->addToVector("sim_hit_x", sim_hit->getPosition()[0]); 
        ntuple_->addToVector("sim_hit_y", sim_hit->getPosition()[1]); 
        ntuple_->addToVector("sim_hit_z", sim_hit->getPosition()[2]); 

        auto value{ EVENT::long64( sim_hit->getCellID0() & 0xffffffff ) | 
            ( EVENT::long64( sim_hit->getCellID1() ) << 32 ) } ; 
        decoder.setValue(value);

        ntuple_->addToVector("sim_hit_layer",  decoder["layer"] ); 
        ntuple_->addToVector("sim_hit_module", decoder["module"] ); 
        ntuple_->addToVector("sim_hit_is_top", isTopLayer(decoder["module"])); 
        ntuple_->addToVector("sim_hit_is_bot", isBottomLayer(decoder["module"]));
        ntuple_->addToVector("sim_hit_is_ele_side",  isElectronSide(decoder["module"])); 
        ntuple_->addToVector("sim_hit_is_pos_side",  isPositronSide(decoder["module"])); 

        auto raw_hit_vec{raw_sim_nav->getRelatedFromObjects(sim_hit)};
        
        EVENT::TrackerRawData* raw_hit_max{nullptr};
        int max_amp{-9999}, strip{-9999}, cluster_size{-9999}; 
        double delta_x{-9999}, delta_y{-9999}, delta_z{-9999};
        double x_err{-9999}, y_err{-9999}, z_err{-9999};  
        long max_value;    
        
        // TODO: Investigate why not all sim hits have raw hits associated with 
        //       them. 
        if (!raw_hit_vec.empty()) {
            for (const auto& hit_obj : raw_hit_vec) { 
        
                auto hit = static_cast<EVENT::TrackerRawData*>(hit_obj); 
            
                std::vector< short > adc_values = hit->getADCValues(); 
                short max{*std::max_element(adc_values.begin(), adc_values.end())}; 
            
                if (max > max_amp) {
                    max_amp = max; 
                    raw_hit_max = hit;
                    max_value = value;  
                }
            }

            auto fields{decodeID(raw_hit_max)};
            strip = std::get< 2 >(fields);  
            
            auto strip_hit_sim{hit_map[raw_hit_max]};
       
            if (strip_hit_sim != nullptr) {
        
                delta_x = strip_hit_sim->getPosition()[0] - sim_hit->getPosition()[0];
                delta_y = strip_hit_sim->getPosition()[1] - sim_hit->getPosition()[1];
                delta_z = strip_hit_sim->getPosition()[2] - sim_hit->getPosition()[2];
                x_err = sqrt(strip_hit_sim->getCovMatrix()[0]); 
                y_err = sqrt(strip_hit_sim->getCovMatrix()[2]); 
                z_err = sqrt(strip_hit_sim->getCovMatrix()[5]); 
                cluster_size = strip_hit_sim->getRawHits().size(); 
            }
        }
        ntuple_->addToVector("sim_hit_raw_strip", strip);
        ntuple_->addToVector("sim_hit_strip_res_x", delta_x); 
        ntuple_->addToVector("sim_hit_strip_res_y", delta_y); 
        ntuple_->addToVector("sim_hit_strip_res_z", delta_z); 
        ntuple_->addToVector("sim_hit_strip_res_xerr", x_err); 
        ntuple_->addToVector("sim_hit_strip_res_yerr", y_err); 
        ntuple_->addToVector("sim_hit_strip_res_zerr", z_err); 
        ntuple_->addToVector("sim_hit_strip_size", cluster_size); 
    }

    ntuple_->fill();

    return true; 
}

void SvtHitProcessor::finalize() { ntuple_->writeTree(); }

bool SvtHitProcessor::isElectronSide(int module) { 
    if (module <= 1) return true;
    return false;  
}

bool SvtHitProcessor::isPositronSide(int module) { 
    if (module >= 2) return true;
    return false;  
}

std::tuple<int, int, int> SvtHitProcessor::decodeID(EVENT::TrackerRawData* hit) { 

    //Decode the cellid
    auto value{ EVENT::long64( hit->getCellID0() & 0xffffffff ) | 
              ( EVENT::long64( hit->getCellID1() ) << 32 ) };
    raw_decoder_.setValue(value);

    return std::make_tuple(raw_decoder_["layer"], raw_decoder_["module"], raw_decoder_["strip"]); 

}

DECLARE_PROCESSOR(SvtHitProcessor)
