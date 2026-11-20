#pragma once

//------------------//
//     C++          //
//------------------//
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

//------------------//
//    hpstr         //
//------------------//
#include "Track.h"

/**
 * Data momentum scale corrections for tracks: a run-by-run scale and, optionally,
 * a mean correction. Either can be switched off by leaving its config empty.
 *
 * Run-by-run scale (runScaleCfg): the "runScale" block of a FEE scale JSON, e.g.
 * analysis/data/smearing/fee_scale_smearing_2021_3p74.json,
 *
 *   "runScale": { "default": {"top": k, "bot": k},
 *                 "runs": { "<run>": {"top": k, "bot": k, ...}, ... } }
 *
 * With half = top if tanLambda > 0 else bot, px, py, pz are multiplied by k and
 * omega divided by k; runs not in the table use runScale.default.
 *
 * Mean correction (meanCorrCfg): a JSON in the TrackSmearingTool schema. It applies
 * the same correction as TrackSmearingTool's data mode, omega' = omega + (mu_mc - mu_data),
 * with the momentum scaled by omega / omega'. With meanCorrVariable empty the unbinned
 * "omegaSmearing" mu_data/mu_mc are used; with "phi0", "tanLambda" or "nHits" the means
 * come from "omegaSmearing_binned_<variable>" (falling back to the unbinned ones if that
 * table has no means), looked up as TrackSmearingTool does.
 *
 * The run scale is applied first, then the mean correction. The tool is meant for
 * data only -- the caller decides when to use it. Smearing is not touched.
 */
class RunScaleCorrectionTool {
  public:
    RunScaleCorrectionTool(const std::string& runScaleCfg, const std::string& meanCorrCfg = "",
                           const std::string& meanCorrVariable = "");

    /** Run-by-run scale factor for this run and detector half (1 if no run scale is configured). */
    double getScale(int run, bool isTop) const;

    /** Apply the configured corrections to the track. Returns the total momentum scale applied. */
    double apply(Track& track, int run) const;

    bool hasRunScale() const { return !runScaleCfg_.empty(); }
    bool hasMeanCorr() const { return !meanCorrCfg_.empty(); }

    /** True if the run has its own entry (false -> the default is used). */
    bool hasRun(int run) const { return scales_.count(run) > 0; }

    void printConfig() const;

  private:
    std::string runScaleCfg_;
    std::string meanCorrCfg_;

    std::map<int, std::pair<double, double>> scales_;  //!< run -> (top, bot)
    double defaultTop_{1.0};
    double defaultBot_{1.0};
    mutable std::set<int> warnedRuns_;  //!< runs already reported as using the default

    /** Track variable the binned means are looked up in (same choices as TrackSmearingTool). */
    double lookupValue(const Track& track) const;
    /** Mean difference mu_mc - mu_data in omega for this track. */
    double omegaMeanShift(const Track& track, bool isTop) const;

    std::string meanCorrVariable_;
    double omegaMuDataTop_{0.}, omegaMuDataBot_{0.};
    double omegaMuMcTop_{0.}, omegaMuMcBot_{0.};
    bool binnedMeans_{false};
    std::vector<double> edgesTop_, muDataTop_, muMcTop_;
    std::vector<double> edgesBot_, muDataBot_, muMcBot_;
};
