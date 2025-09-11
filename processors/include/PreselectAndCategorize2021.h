/*
 * @file PreselectAndCategorize.h
 * @author Sarah Gaiser
 * @date Apr 2025
 */

#ifndef __PRESELECT_AND_CATEGORIZE_2021_H__
#define __PRESELECT_AND_CATEGORIZE_2021_H__

#include "PreselectAndCategorize.h"
#include "TrackSmearingTool.h"
#include "TSData.h"


class PreselectAndCategorize2021 : public PreselectAndCategorize {
    public:
        PreselectAndCategorize2021(const std::string& name, Process& process):
            PreselectAndCategorize(name,process) {}
        ~PreselectAndCategorize2021() = default;
        virtual void configure(const ParameterSet& parameters) final;
        virtual void initialize(TTree* tree) final;
        virtual void setFile(TFile* out_file) final;
        virtual bool process(IEvent* ievent) final;
        virtual void finalize() final;

    private:
        EventBus bus_;
        std::unique_ptr<TTree> output_tree_;

        std::string vtxColl_{"UnconstrainedV0Vertices_KF"};
        std::string mcColl_{"MCParticle"}; //!< description
        std::string trkColl_{"KalmanFullTracks"};
        bool isData_{false};
        bool isSimpSignal_{false};
        bool isApSignal_{false};
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

#endif //__PRESELECT_AND_CATEGORIZE_2021_H__