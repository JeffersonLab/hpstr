#include "RunScaleCorrectionTool.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "json.hpp"
using json = nlohmann::json;

namespace {
json readJson(const std::string& file) {
    std::ifstream ifs(file);
    if (!ifs.good()) throw std::runtime_error("[RunScaleCorrectionTool] cannot open " + file);
    json cfg;
    ifs >> cfg;
    return cfg;
}

// Same lookup as TrackSmearingTool::lookupBinnedValue: clamp to the first/last bin.
double lookupBinned(const std::vector<double>& edges, const std::vector<double>& values, double x) {
    if (edges.size() < 2 || values.empty()) return 0.;
    if (x <= edges.front()) return values.front();
    if (x >= edges.back()) return values.back();
    int bin = static_cast<int>(std::distance(edges.begin(), std::upper_bound(edges.begin(), edges.end(), x))) - 1;
    bin = std::max(0, std::min(bin, static_cast<int>(values.size()) - 1));
    return values[bin];
}
}  // namespace

RunScaleCorrectionTool::RunScaleCorrectionTool(const std::string& runScaleCfg, const std::string& meanCorrCfg,
                                               const std::string& meanCorrVariable)
    : runScaleCfg_(runScaleCfg), meanCorrCfg_(meanCorrCfg), meanCorrVariable_(meanCorrVariable) {
    if (!runScaleCfg_.empty()) {
        json cfg = readJson(runScaleCfg_);
        if (!cfg.contains("runScale"))
            throw std::runtime_error("[RunScaleCorrectionTool] no 'runScale' block in " + runScaleCfg_);
        const json& rs = cfg["runScale"];
        if (rs.contains("default")) {
            defaultTop_ = rs["default"].value("top", 1.0);
            defaultBot_ = rs["default"].value("bot", 1.0);
        }
        if (rs.contains("runs")) {
            for (auto it = rs["runs"].begin(); it != rs["runs"].end(); ++it) {
                const json& entry = it.value();
                if (!entry.contains("top") || !entry.contains("bot"))
                    throw std::runtime_error("[RunScaleCorrectionTool] run " + it.key() + " lacks a top/bot scale in " +
                                             runScaleCfg_);
                scales_[std::stoi(it.key())] = {entry["top"].get<double>(), entry["bot"].get<double>()};
            }
        }
    }

    if (!meanCorrCfg_.empty()) {
        json cfg = readJson(meanCorrCfg_);
        if (!cfg.contains("omegaSmearing") || !cfg["omegaSmearing"].contains("mu_data") ||
            !cfg["omegaSmearing"].contains("mu_mc"))
            throw std::runtime_error("[RunScaleCorrectionTool] no omegaSmearing.mu_data / mu_mc in " + meanCorrCfg_);
        const json& os = cfg["omegaSmearing"];
        omegaMuDataTop_ = os["mu_data"]["top"].get<double>();
        omegaMuDataBot_ = os["mu_data"]["bot"].get<double>();
        omegaMuMcTop_ = os["mu_mc"]["top"].get<double>();
        omegaMuMcBot_ = os["mu_mc"]["bot"].get<double>();

        if (!meanCorrVariable_.empty()) {
            const std::string key = "omegaSmearing_binned_" + meanCorrVariable_;
            if (cfg.contains(key) && cfg[key]["top"].contains("mu_data") && cfg[key]["top"].contains("mu_mc")) {
                const json& b = cfg[key];
                edgesTop_ = b["top"]["bin_edges"].get<std::vector<double>>();
                muDataTop_ = b["top"]["mu_data"].get<std::vector<double>>();
                muMcTop_ = b["top"]["mu_mc"].get<std::vector<double>>();
                edgesBot_ = b["bot"]["bin_edges"].get<std::vector<double>>();
                muDataBot_ = b["bot"]["mu_data"].get<std::vector<double>>();
                muMcBot_ = b["bot"]["mu_mc"].get<std::vector<double>>();
                binnedMeans_ = true;
            } else {
                std::cout << "[RunScaleCorrectionTool] WARNING: no binned means in " << key
                          << "; using the unbinned omegaSmearing means" << std::endl;
            }
        }
    }
}

double RunScaleCorrectionTool::lookupValue(const Track& track) const {
    if (meanCorrVariable_ == "phi0" || meanCorrVariable_ == "phi") return track.getPhi();
    if (meanCorrVariable_ == "nHits" || meanCorrVariable_ == "nhits") return track.getTrackerHitCount();
    return track.getTanLambda();
}

double RunScaleCorrectionTool::omegaMeanShift(const Track& track, bool isTop) const {
    if (binnedMeans_) {
        const double x = lookupValue(track);
        return isTop ? lookupBinned(edgesTop_, muMcTop_, x) - lookupBinned(edgesTop_, muDataTop_, x)
                     : lookupBinned(edgesBot_, muMcBot_, x) - lookupBinned(edgesBot_, muDataBot_, x);
    }
    return isTop ? (omegaMuMcTop_ - omegaMuDataTop_) : (omegaMuMcBot_ - omegaMuDataBot_);
}

double RunScaleCorrectionTool::getScale(int run, bool isTop) const {
    if (runScaleCfg_.empty()) return 1.0;
    auto it = scales_.find(run);
    if (it != scales_.end()) return isTop ? it->second.first : it->second.second;
    if (warnedRuns_.insert(run).second)
        std::cout << "[RunScaleCorrectionTool] WARNING: run " << run << " not in " << runScaleCfg_
                  << "; using the default scale (top " << defaultTop_ << ", bot " << defaultBot_ << ")" << std::endl;
    return isTop ? defaultTop_ : defaultBot_;
}

double RunScaleCorrectionTool::apply(Track& track, int run) const {
    const bool isTop = track.getTanLambda() > 0;
    const double omega = track.getOmega();
    double scale = getScale(run, isTop);
    double omegaCorr = omega / scale;

    if (!meanCorrCfg_.empty()) {
        // Same as TrackSmearingTool's data-mode omega mean correction
        omegaCorr += omegaMeanShift(track, isTop);
        if (omegaCorr != 0.) scale = omega / omegaCorr;
    }

    if (scale == 1.0) return scale;
    std::vector<double> mom = track.getMomentum();
    for (double& c : mom) c *= scale;
    track.setMomentum(mom);
    track.setOmega(omega / scale);
    return scale;
}

void RunScaleCorrectionTool::printConfig() const {
    if (runScaleCfg_.empty())
        std::cout << "[RunScaleCorrectionTool] run-by-run scale: off" << std::endl;
    else
        std::cout << "[RunScaleCorrectionTool] run-by-run scale: " << runScaleCfg_ << " (" << scales_.size()
                  << " runs, default top " << defaultTop_ << " bot " << defaultBot_ << ")" << std::endl;
    if (meanCorrCfg_.empty())
        std::cout << "[RunScaleCorrectionTool] mean correction: off" << std::endl;
    else
        std::cout << "[RunScaleCorrectionTool] mean correction: " << meanCorrCfg_
                  << (binnedMeans_ ? " (binned in " + meanCorrVariable_ + ")" : " (unbinned)") << std::endl;
}
