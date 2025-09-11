/*
 * @file PreselectAndCategorize.h
 * @author Sarah Gaiser adapting Tom Eichlersmith's PreselectAndCategorize
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
        std::string trkColl_{"KalmanFullTracks"};
        bool isSimpSignal_{false};
        bool isApSignal_{false};
};

#endif //__PRESELECT_AND_CATEGORIZE_2021_H__