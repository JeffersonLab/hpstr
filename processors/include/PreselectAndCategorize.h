/*
 * @file PreselectAndCategorize.h
 * @author Tom Eichlersmith
 * @date Dec 2024
 * @author Sarah Gaiser
 * @date Apr 2025
 */

#ifndef __PRESELECT_AND_CATEGORIZE_H__
#define __PRESELECT_AND_CATEGORIZE_H__

// HPSTR
#include "HpsEvent.h"
#include "Collections.h"
#include "EventHeader.h"
#include "TSData.h"
#include "Vertex.h"
#include "Track.h"
#include "TrackerHit.h"
#include "MCParticle.h"
#include "Particle.h"
#include "Processor.h"
#include "BaseSelector.h"
#include "TrackHistos.h"
#include "MCAnaHistos.h"
#include "utilities.h"

#include "TrackSmearingTool.h"
#include "FlatTupleMaker.h"
#include "AnaHelpers.h"

#include "Bus.h"

// ROOT
#include "TFile.h"
#include "TTree.h"
#include "TRefArray.h"
#include "TBranch.h"
#include "TVector3.h"
#include "TLorentzVector.h"
#include "TAxis.h"

// C++ 
#include <memory>
#include <iostream>
#include <fstream>
#include <map>


// json json_load(const std::string& filepath) {
//     json obj;
//     std::ifstream file{filepath};
//     file >> obj;
//     file.close();
//     return obj;
// }


class Cutflow {
    
    public:
        Cutflow(const std::string& name, const std::string& nocut_name):
            name_{name}, nocut_name_{nocut_name} {}

        void set_label_names(const std::vector<std::string>& label_names) {
            label_names_ = label_names;
        }
        
        void add(const std::string& name, int nbins, float min, float max) {
            if (h_cutflow_) {
                throw std::runtime_error("Cannot add more cuts after init");
            }
            
            auto h = std::make_unique<TH1F>(
                        ("nm1_" + name + "_h").c_str(),
                        (name + " N-1").c_str(),
                        nbins, min, max);
            
            cuts_.emplace(name, std::make_pair<std::size_t, std::unique_ptr<TH1F>>(cuts_.size(), std::move(h)));
        }

        void init() {
            if (h_cutflow_) {
                throw std::runtime_error("Cannot init twice.");
            }
            
            h_cutflow_ = std::make_unique<TH1F>(
                (name_ + "_cutflow_h").c_str(),
                (name_ + " cutflow;;N_{events}").c_str(),
                cuts_.size()+1, -1.5, cuts_.size()-0.5
            );
            
            cut_desc_.resize(cuts_.size());
        }

        void begin_event() {
            h_cutflow_->Fill(nocut_name_.c_str(), 1.);
            keep_ = true;
            for (auto& d : cut_desc_) d.reset();
        }

        void apply(const std::string& name, bool descision) {
            if (cuts_.find(name) == cuts_.end()) {
                throw std::runtime_error("Cut "+name+" not `add`ed to cutflow.");
            }

            keep_ = (keep_ and descision);
            if (keep_) h_cutflow_->Fill(name.c_str(), 1.);
            int i_cut = cuts_.at(name).first;
            cut_desc_[i_cut] = descision;
        }

        template<typename T>
        void fill_nm1(const std::string& name, T value) {
            bool should_fill{true};

            for (std::size_t i_cut{0}; i_cut < cuts_.size(); i_cut++) {
                if (i_cut != cuts_[name].first and cut_desc_[i_cut].has_value()) {
                    should_fill = (should_fill and cut_desc_[i_cut].value());
                }
            }

            if (should_fill) {
                cuts_.at(name).second->Fill(value);
            }
        }

        bool keep() const {
            return keep_;
        }

        void save() {
            // if label names not set or size mismatch, just skip setting labels
            for (int i = 0; i < h_cutflow_->GetNbinsX(); ++i) {
                if (label_names_.size() == (std::size_t)h_cutflow_->GetNbinsX()) {
                    h_cutflow_->GetXaxis()->SetBinLabel(i+1, label_names_.at(i).c_str());
                }
            }
            h_cutflow_->Write();
            for (const auto& [_name, entry] : cuts_) entry.second->Write();
        }

    private:
        std::string name_, nocut_name_;
        std::unique_ptr<TH1F> h_cutflow_;
        std::map<std::string,std::pair<std::size_t,std::unique_ptr<TH1F>>> cuts_;
        bool keep_;
        std::vector<std::optional<bool>> cut_desc_;
        std::vector<std::string> label_names_;
};

class EventBus {
    framework::Bus bus_;

    public:
        bool has(const std::string& name) {
            return bus_.isOnBoard(name);
        }

        template<typename BaggageType>
        void board_input(TTree* tree, const std::string& name) {
            if (bus_.isOnBoard(name)) {
                throw std::runtime_error(name + " is already on board the bus.");
            }

            bus_.board<BaggageType>(name);
            if (bus_.attach(tree, name, false /* create if missing */) == 0) {
                throw std::runtime_error("Unable to find branch " + name + " in input TTree.");
            }
        }

        template<typename BaggageType>
        void board_output(TTree* tree, const std::string& name) {
            if (bus_.isOnBoard(name)) {
                throw std::runtime_error(name + " is already on board the bus.");
            }
            bus_.board<BaggageType>(name);
            if (bus_.attach(tree, name, true /* create if missing */) == 0) {
                throw std::runtime_error("Unable to create branch " + name + " in output TTree.");
            }
        }

        template<typename BaggageType>
        const BaggageType& get(const std::string& name) {
            return bus_.get<BaggageType>(name);
        }

        template<typename BaggageType>
        void set(const std::string& name, const BaggageType& obj) {
            if (bus_.isOnBoard(name)) {
                bus_.update(name, obj);
            return;
            }
            throw std::runtime_error("No branch " + name + " boarded on the bus.");
        }
};

class PreselectAndCategorize : public Processor {
    public:
        PreselectAndCategorize(const std::string& name, Process& process):
            Processor(name,process) {}
        ~PreselectAndCategorize() = default;
        virtual void configure(const ParameterSet& parameters);
        virtual void initialize(TTree* tree);
        virtual void setFile(TFile* out_file);
        virtual bool process(IEvent* ievent);
        virtual void finalize();

        virtual void setVtxColl(std::string vtx_coll) {
            vtxColl_ = vtx_coll;
        }
        virtual void setMCColl(std::string mc_coll) {
            mcColl_ = mc_coll;
        }

        json preselect_json_load(const std::string& filepath) {
            json obj;
            std::ifstream file{filepath};
            file >> obj;
            file.close();
            return obj;
        }      
        
        bool getIsSignal() const {
            return isSignal_;
        }

        bool getIsData() const {
            return isData_;
        }

    private:
        EventBus bus_;
        std::unique_ptr<TTree> output_tree_;

        std::string vtxColl_{"UnconstrainedV0Vertices_KF"};
        std::string mcColl_{"MCParticle"}; //!< description
        bool isData_{false};
        bool isSignal_{false};
        double calTimeOffset_{0.0};

        Cutflow vertex_cf_{"vertex", "reconstructed"};
        Cutflow event_cf_{"event", "readout"};
        std::unique_ptr<TH2F> n_vertices_h_;
        std::shared_ptr<TrackSmearingTool> smearingTool_;
        std::shared_ptr<AnaHelpers> _ah; //!< description

        /**
        * Map of beamspot by run, currently not used but available I guess??
        */
        std::map<int,std::vector<double>> beamspot_corrections_;

        // config for projecting vertex candidates back to target
        json v0proj_fits_;

        /**
        * Map of correction name to correction value for analysis-level
        * corrections on tracks.
        */
        std::map<std::string,double> track_corrections_;
};

#endif //__PRESELECT_AND_CATEGORIZE_H__
