#include "TrackCorrector.h"

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "TVector3.h"

using json = nlohmann::json;

namespace {
json readJson(const std::string& file) {
    std::ifstream ifs(file);
    if (!ifs.good()) throw std::runtime_error("[TrackCorrector] cannot open " + file);
    json cfg;
    ifs >> cfg;
    return cfg;
}
}  // namespace

std::string TrackCorrector::resolvePath(const std::string& path) const {
    if (path.empty() || path.front() == '/') return path;
    const char* base = std::getenv("HPSTR_BASE");
    if (!base) throw std::runtime_error("[TrackCorrector] relative path " + path + " but HPSTR_BASE is not set");
    return std::string(base) + "/" + path;
}

std::string TrackCorrector::dataOrMc(const json& block) const {
    const char* key = opts_.isData ? "data" : "mc";
    if (!block.is_object() || !block.contains(key)) return "";
    return resolvePath(block[key].get<std::string>());
}

TrackCorrector::TrackCorrector(const std::string& cfgFile, const Options& opts)
    : cfgFile_(resolvePath(cfgFile)), opts_(opts) {
    json cfg = readJson(cfgFile_);

    // Constant track offsets. Track::applyCorrection silently ignores unknown names,
    // so check the spelling here.
    std::string offsets = resolvePath(cfg.value("trackOffsets", ""));
    if (!offsets.empty()) {
        for (const auto& [name, corr] : readJson(offsets).items()) {
            if (name != "track_z0" && name != "track_time")
                throw std::runtime_error("[TrackCorrector] unrecognized track offset '" + name + "' in " + offsets);
            trackOffsets_[name] = corr.get<double>();
        }
    }

    if (cfg.contains("z0Offset")) {
        z0OffsetFile_ = dataOrMc(cfg["z0Offset"]);
        if (!z0OffsetFile_.empty()) {
            z0OffsetFits_ = readJson(z0OffsetFile_);
            if (z0OffsetFits_.empty()) throw std::runtime_error("[TrackCorrector] no runs in " + z0OffsetFile_);
        }
    }

    if (cfg.contains("z0Calib")) {
        z0CalibFile_ = dataOrMc(cfg["z0Calib"]);
        if (!z0CalibFile_.empty()) {
            z0Calib_ = std::make_shared<TrackBiasingTool>("", "KalmanFullTracks");
            z0Calib_->loadZ0Calibration(z0CalibFile_);
            z0Calib_->setDebug(opts_.debug);
        }
    }

    if (cfg.contains("momentumScale") && opts_.isData) {
        const json& ms = cfg["momentumScale"];
        std::string runScale = resolvePath(ms.value("runScale", ""));
        std::string meanCorr = resolvePath(ms.value("meanCorr", ""));
        if (!runScale.empty() || !meanCorr.empty())
            momentumScale_ =
                std::make_shared<RunScaleCorrectionTool>(runScale, meanCorr, ms.value("meanCorrVariable", ""));
    }

    if (cfg.contains("smearing")) {
        const json& sm = cfg["smearing"];
        std::string smearCfg = resolvePath(sm.value("cfg", ""));
        if (!smearCfg.empty()) {
            // relSmearingP=true (relative), relSmearingZ0=false (absolute); a JSON smearing
            // file overrides these with its own values.
            smearing_ = std::make_shared<TrackSmearingTool>(smearCfg, true, false, opts_.seed,
                                                            sm.value("trackCollection", "KalmanFullTracks"),
                                                            sm.value("smearingFactor", 1.0));
            smearing_->setForcedVariable(sm.value("variable", ""));
            bool truth = opts_.requireTruthMatch >= 0 ? opts_.requireTruthMatch != 0
                                                      : sm.value("requireTruthMatch", false);
            smearing_->setRequireTruthMatch(truth);
            smearing_->setDebug(opts_.debug);
            smearing_->setIsData(opts_.isData);
            // Data scale corrections come only from momentumScale, never from the smearing tool.
            smearing_->setApplyMeanCorr(false);
            smearOmega_ = sm.value("smearOmega", false);
            smearZ0_ = sm.value("smearZ0", true);
        }
    }
    applySmearing_ = smearing_ && !opts_.isData && opts_.doSmearing;

    // Vertex momenta follow the tracks whenever a momentum step can run. With a smearing
    // file on data the vertex is still refreshed (ratios of 1), as the preselection always did.
    updatesVertex_ = (smearing_ && (opts_.isData || opts_.doSmearing)) || momentumScale_;
}

void TrackCorrector::beginEvent(int run, const std::vector<MCParticle*>* mcParticles) {
    run_ = run;
    if (z0Calib_) z0Calib_->setRun(run);
    if (smearing_ && mcParticles) smearing_->setMCParticles(mcParticles);

    if (!z0OffsetFile_.empty() && run != z0OffsetRun_) {
        // Closest run at or below this one; runs before every entry use the earliest one.
        int closest = -1;
        for (const auto& entry : z0OffsetFits_.items()) {
            int check = std::stoi(entry.key());
            if (check > run) {
                if (closest < 0) closest = check;
                break;
            }
            closest = check;
        }
        const json& fit = z0OffsetFits_[std::to_string(closest)];
        eleZ0Offset_ = fit["elez0_mean"].get<double>();
        posZ0Offset_ = fit["posz0_mean"].get<double>();
        z0OffsetRun_ = run;
    }
}

std::vector<TrackCorrector::Result> TrackCorrector::correct(const std::vector<Track*>& tracks,
                                                            const std::vector<int>& charges) {
    std::vector<Result> results(tracks.size());
    if (!charges.empty() && charges.size() != tracks.size())
        throw std::runtime_error("[TrackCorrector] charges and tracks differ in size");

    for (Track* trk : tracks)
        for (const auto& [name, corr] : trackOffsets_) trk->applyCorrection(name, corr);

    if (!z0OffsetFile_.empty()) {
        for (std::size_t i = 0; i < tracks.size(); ++i) {
            int charge = charges.empty() ? tracks[i]->getCharge() : charges[i];
            tracks[i]->applyCorrection("z0", charge < 0 ? eleZ0Offset_ : posZ0Offset_);
        }
    }

    if (z0Calib_)
        for (Track* trk : tracks) z0Calib_->updateWithCalibrateZ0(*trk);

    if (momentumScale_)
        for (std::size_t i = 0; i < tracks.size(); ++i) results[i].pScale = momentumScale_->apply(*tracks[i], run_);

    if (applySmearing_) {
        if (smearZ0_)
            for (Track* trk : tracks) smearing_->updateWithSmearZ0(*trk);
        for (std::size_t i = 0; i < tracks.size(); ++i)
            results[i].pSmear = smearOmega_ ? smearing_->updateWithSmearOmega(*tracks[i])
                                            : smearing_->updateWithSmearP(*tracks[i]);
    }
    return results;
}

void TrackCorrector::correctVertex(Vertex* vtx, double r1, double r2) {
    TVector3 p1(vtx->getP1X() * r1, vtx->getP1Y() * r1, vtx->getP1Z() * r1);
    TVector3 p2(vtx->getP2X() * r2, vtx->getP2Y() * r2, vtx->getP2Z() * r2);
    vtx->setVtxParameters(p1, p2, vtx->getInvMass() * std::sqrt(r1 * r2));
}

void TrackCorrector::printConfig() const {
    std::cout << "[TrackCorrector] " << cfgFile_ << " (" << (opts_.isData ? "data" : "MC") << ")" << std::endl;
    std::cout << "  trackOffsets : " << (trackOffsets_.empty() ? "off" : "") ;
    for (const auto& [name, corr] : trackOffsets_) std::cout << name << "=" << corr << " ";
    std::cout << std::endl;
    std::cout << "  z0Offset     : " << (z0OffsetFile_.empty() ? "off" : z0OffsetFile_) << std::endl;
    std::cout << "  z0Calib      : " << (z0CalibFile_.empty() ? "off" : z0CalibFile_) << std::endl;
    std::cout << "  momentumScale: " << (momentumScale_ ? "on (data)" : "off") << std::endl;
    if (momentumScale_) momentumScale_->printConfig();
    std::cout << "  smearing     : "
              << (!smearing_ ? "off" : applySmearing_ ? (smearOmega_ ? "on (omega" : "on (p") : "configured, not applied (")
              << (smearing_ ? std::string(smearZ0_ ? ", z0" : ", no z0") + ")" : "") << std::endl;
    if (smearing_) smearing_->printConfig();
}
