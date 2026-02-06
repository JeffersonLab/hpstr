/*
 * @file PreselectAndCategorize.h
 * @author Tom Eichlersmith
 * @date Dec 2024
 * @author Sarah Gaiser
 * @date Apr 2025
 */

#ifndef __PRESELECT_AND_CATEGORIZE_SIMP_2016_H__
#define __PRESELECT_AND_CATEGORIZE_SIMP_2016_H__

#include "PreselectAndCategorize.h"
#include "TrackSmearingTool.h"


class PreselectAndCategorizeSimp2016 : public PreselectAndCategorize {
    public:
        PreselectAndCategorizeSimp2016(const std::string& name, Process& process):
            PreselectAndCategorize(name,process) {}
        ~PreselectAndCategorizeSimp2016() = default;
        virtual void configure(const ParameterSet& parameters) final;
        virtual void initialize(TTree* tree) final;
        virtual void setFile(TFile* out_file) final;
        virtual bool process(IEvent* ievent) final;
        virtual void finalize() final;
};

#endif //__PRESELECT_AND_CATEGORIZE_SIMP_2016_H__