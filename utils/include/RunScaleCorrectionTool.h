#pragma once

//------------------//
//     C++          //
//------------------//
#include <map>
#include <set>
#include <string>
#include <utility>

//------------------//
//    hpstr         //
//------------------//
#include "Track.h"

/**
 * Run-by-run track momentum scale correction for data.
 *
 * Reads the "runScale" block of a FEE scale JSON (e.g.
 * analysis/data/smearing/fee_scale_smearing_2021_3p74.json):
 *
 *   "runScale": { "default": {"top": k, "bot": k},
 *                 "runs": { "<run>": {"top": k, "bot": k, ...}, ... } }
 *
 * For each track, half = top if tanLambda > 0 else bot, and k is taken from the
 * run's entry (runScale.default for runs not in the table). apply() multiplies
 * px, py, pz by k and divides omega by k. Only the scale is read; any smearing
 * block in the same file is ignored, so this tool is independent of
 * TrackSmearingTool. It is meant for data only -- the caller decides when to use it.
 */
class RunScaleCorrectionTool {
  public:
    explicit RunScaleCorrectionTool(const std::string& cfgFile);

    /** Scale factor for this run and detector half. */
    double getScale(int run, bool isTop) const;

    /** Scale the track's momentum by k (omega by 1/k). Returns the k applied. */
    double apply(Track& track, int run) const;

    /** True if the run has its own entry (false -> the default is used). */
    bool hasRun(int run) const { return scales_.count(run) > 0; }

    void printConfig() const;

  private:
    std::string cfgFile_;
    std::map<int, std::pair<double, double>> scales_;  //!< run -> (top, bot)
    double defaultTop_{1.0};
    double defaultBot_{1.0};
    mutable std::set<int> warnedRuns_;  //!< runs already reported as using the default
};
