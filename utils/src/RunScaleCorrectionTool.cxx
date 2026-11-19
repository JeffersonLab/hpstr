#include "RunScaleCorrectionTool.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "json.hpp"
using json = nlohmann::json;

RunScaleCorrectionTool::RunScaleCorrectionTool(const std::string& cfgFile) : cfgFile_(cfgFile) {
    std::ifstream ifs(cfgFile_);
    if (!ifs.good()) throw std::runtime_error("[RunScaleCorrectionTool] cannot open " + cfgFile_);
    json cfg;
    ifs >> cfg;
    if (!cfg.contains("runScale")) throw std::runtime_error("[RunScaleCorrectionTool] no 'runScale' block in " + cfgFile_);

    const json& rs = cfg["runScale"];
    if (rs.contains("default")) {
        defaultTop_ = rs["default"].value("top", 1.0);
        defaultBot_ = rs["default"].value("bot", 1.0);
    }
    if (rs.contains("runs")) {
        for (auto it = rs["runs"].begin(); it != rs["runs"].end(); ++it) {
            const json& entry = it.value();
            if (!entry.contains("top") || !entry.contains("bot"))
                throw std::runtime_error("[RunScaleCorrectionTool] run " + it.key() + " lacks a top/bot scale in " + cfgFile_);
            scales_[std::stoi(it.key())] = {entry["top"].get<double>(), entry["bot"].get<double>()};
        }
    }
}

double RunScaleCorrectionTool::getScale(int run, bool isTop) const {
    auto it = scales_.find(run);
    if (it != scales_.end()) return isTop ? it->second.first : it->second.second;
    if (warnedRuns_.insert(run).second)
        std::cout << "[RunScaleCorrectionTool] WARNING: run " << run << " not in " << cfgFile_
                  << "; using the default scale (top " << defaultTop_ << ", bot " << defaultBot_ << ")" << std::endl;
    return isTop ? defaultTop_ : defaultBot_;
}

double RunScaleCorrectionTool::apply(Track& track, int run) const {
    const double k = getScale(run, track.getTanLambda() > 0);
    std::vector<double> mom = track.getMomentum();
    for (double& c : mom) c *= k;
    track.setMomentum(mom);
    track.setOmega(track.getOmega() / k);
    return k;
}

void RunScaleCorrectionTool::printConfig() const {
    std::cout << "[RunScaleCorrectionTool] " << cfgFile_ << ": " << scales_.size()
              << " runs, default top " << defaultTop_ << " bot " << defaultBot_ << std::endl;
}
