#pragma once

#include <vector>
#include <map>
#include <iostream>
#include <cmath>

#include "Track.h"
#include "MCParticle.h"
#include "TrackerHit.h"
#include "TRefArray.h"

namespace utils {

/**
 * @brief Get the PDG code of the truth particle best matching this track
 *
 * Finds the MC particle with the most hits on the track using two methods:
 * 1. getMcpHits() from Track (populated by VertexProcessor)
 * 2. getSvtHits() and getMCPartIDs() from TrackerHits (fallback)
 *
 * @param trk The track to match
 * @param mcParticles Pointer to the MC particle collection
 * @param debug Enable debug output
 * @return PDG code of best-matching particle, or 0 if no match found
 */
inline int getTruthPDG(Track& trk, const std::vector<MCParticle*>* mcParticles, bool debug = false) {
    if (!mcParticles || mcParticles->empty()) {
        if (debug) {
            std::cout << "utils::getTruthPDG: No MC particles available" << std::endl;
        }
        return 0;
    }

    // Try two methods to find the MC particle ID with the most hits on this track:
    // Method 1: Use getMcpHits() from Track (populated by VertexProcessor)
    // Method 2: Use getSvtHits() and getMCPartIDs() from TrackerHits

    std::map<int, int> count_per_particle_id;

    // Method 1: Try getMcpHits() first
    auto mcp_hits = trk.getMcpHits();
    if (debug) {
        std::cout << "utils::getTruthPDG: Method 1 - track has " << mcp_hits.size() << " MCP hits" << std::endl;
    }

    for (const auto& [layer_id, particle_id] : mcp_hits) {
        count_per_particle_id[particle_id]++;
    }

    // Method 2: If no MCP hits, try getting MC info from TrackerHits via getSvtHits()
    if (count_per_particle_id.empty()) {
        TRefArray svt_hits = trk.getSvtHits();
        if (debug) {
            std::cout << "utils::getTruthPDG: Method 2 - track has " << svt_hits.GetEntries() << " SVT hits" << std::endl;
        }

        for (int i = 0; i < svt_hits.GetEntries(); i++) {
            TrackerHit* hit = static_cast<TrackerHit*>(svt_hits.At(i));
            if (hit) {
                std::vector<int> mc_part_ids = hit->getMCPartIDs();
                for (int part_id : mc_part_ids) {
                    count_per_particle_id[part_id]++;
                }
            }
        }
    }

    if (debug) {
        std::cout << "utils::getTruthPDG: found " << count_per_particle_id.size() << " unique particle IDs" << std::endl;
    }

    // Find particle with most hits
    int truth_id{-1}, max_nhits{0};
    for (const auto& [particle_id, count] : count_per_particle_id) {
        if (count > max_nhits) {
            truth_id = particle_id;
            max_nhits = count;
        }
    }

    if (truth_id < 0) {
        if (debug) {
            std::cout << "utils::getTruthPDG: No hits found, returning 0" << std::endl;
        }
        return 0;
    }

    // Find the MC particle with this ID and return its PDG
    for (MCParticle* ptr : *mcParticles) {
        if (ptr->getID() == truth_id) {
            if (debug) {
                std::cout << "utils::getTruthPDG: Match found, PDG=" << ptr->getPDG() << std::endl;
            }
            return ptr->getPDG();
        }
    }

    if (debug) {
        std::cout << "utils::getTruthPDG: No MC particle found with ID " << truth_id << std::endl;
    }
    return 0;
}

/**
 * @brief Check if a track has a truth-matched electron/positron
 *
 * @param trk The track to check
 * @param mcParticles Pointer to the MC particle collection
 * @param debug Enable debug output
 * @return true if track matches an electron or positron (abs(PDG) == 11)
 */
inline bool hasTruthMatch(Track& trk, const std::vector<MCParticle*>* mcParticles, bool debug = false) {
    int pdg = getTruthPDG(trk, mcParticles, debug);
    return (std::abs(pdg) == 11);
}

}  // namespace utils
