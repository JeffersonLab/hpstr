import HpstrConf
import os
import sys
import baseConfig as base

options = base.parser.parse_args()

# Use the input file to set the output file name
inFilename   = options.inFilename
outFilename  = options.outFilename

# The seed is the file number.
smearingSeed = options.seed

print('Input file:  %s' % inFilename)
print('Output file: %s' % outFilename)

p = HpstrConf.Process()

p.run_mode = 1
p.skip_events = options.skip_events
p.max_events = options.nevents


p.add_library("libprocessors")

anaTrks = HpstrConf.Processor('anaTrks', 'FeeAnaProcessor')
anaTrks.parameters["debug"] = 0
anaTrks.parameters["seed"] = smearingSeed
anaTrks.parameters["trkCollName"] = 'KalmanFullTracks'
# FEE tracks come from the FinalStateParticles, where the track and its ECal cluster
# are already associated at the LCIO stage (no manual track-cluster matching).
anaTrks.parameters["fspCollName"] = 'FinalStateParticles_KF'
anaTrks.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/fee_smearing/feeSmearing_2021.json'
anaTrks.parameters["selectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/trackHit/trackHitAna.json'
anaTrks.parameters["isData"] = options.isData

# FEE (Full Energy Electron) selection
anaTrks.parameters["requireElectron"] = 1     # track charge < 0
anaTrks.parameters["requireCluster"]  = 1     # require an associated full-energy cluster
# Require the FEE trigger (TSBank FEE_Top||FEE_Bot, 2600-5200 MeV). Applied to data
# only inside the processor (MC TSBank bits are not filled).
anaTrks.parameters["requireFeeTrigger"] = 1
anaTrks.parameters["feeClusterEnergyMin"] = 3.0  # ~0.80 * 3.74 GeV beam
# E/p window (off by default: [0, 99]); tighten once the ECal energy scale is calibrated
anaTrks.parameters["eopMin"] = 0.0
anaTrks.parameters["eopMax"] = 99.0

# Cluster-time offset: matches the value used by the 2021 preselection
# (PreselectAndCategorize2021, calTimeOffset = 37.3 ns, same for data and MC).
# The window below is applied to the OFFSET-CORRECTED cluster time
# (cluster.getTime() - CalTimeOffset), which is centered near 0.
anaTrks.parameters["CalTimeOffset"]   = 37.3
anaTrks.parameters["CalTimeOffsetMC"] = 37.3
anaTrks.parameters["clusterTimeMin"]   = -6.0
anaTrks.parameters["clusterTimeMax"]   =  6.0
anaTrks.parameters["clusterTimeMinMC"] = -6.0
anaTrks.parameters["clusterTimeMaxMC"] =  6.0

# In-time isolation veto: a clean FEE has a single track and a single cluster.
# Veto the candidate if any additional cluster/track is in-time with the FEE cluster.
# (The n_clusters_intime / n_tracks_intime branches record the counts for tuning.)
#anaTrks.parameters["vetoExtraClusters"]    = 1
#anaTrks.parameters["vetoExtraTracks"]      = 1
#anaTrks.parameters["vetoClusterTimeWindow"] = 8.0     # |t_clu - t_FEEclu| (ns)
#anaTrks.parameters["vetoTrackTimeWindow"]   = 8.0     # |t_trk - t_FEEclu(corrected)| (ns)
#anaTrks.parameters["vetoClusterEnergyMin"]  = 0.5     # only clusters above 0.5 GeV count

#SmearingClosureTest
anaTrks.parameters["smearingCfg"] = os.environ['HPSTR_BASE']+"/analysis/data/smearing/tool_smearing.json"
anaTrks.parameters["doSmearing"] = 1 #0 if options.isData else 1
anaTrks.parameters["smearingFactor"] = 1.0
anaTrks.parameters["smearOmega"] = 1
#anaTrks.parameters["smearingVariable"] = "tanLambda"
anaTrks.parameters["smearingVariable"] = "phi0"
#anaTrks.parameters["smearingVariable"] = "flat"
# Scale correction: "" = use omega-space means (default); "tanLambda" = p-space means
anaTrks.parameters["scaleCorrVariable"] = ""

# Per-hit_pattern momentum scale correction (data momentum -> beam energy).
# p_corr = p * scale, keyed by hit_pattern; default_scale for unlisted patterns.
# Output branches: track_corr.*, hit_pattern_scale, hit_pattern_scale_isdefault.
anaTrks.parameters["doHitPatternScale"] = 1
anaTrks.parameters["hitPatternScaleFile"] = '/sdf/data/hps/users/mgignac/software/2021-ana/plotting/plots/track_smearing_hitpattern_summary/hitpattern_scale_corrections.json'
anaTrks.parameters["hitPatternScaleDataOnly"] = 1  # apply on data only; MC left untouched

# Data momentum scale corrections, applied to track_smeared (the same collection the MC
# smearing goes into); track stays raw. The total scale applied is stored in "run_scale".
#  runScaleCfg: run-by-run FEE scale, p -> k*p and omega -> omega/k per run and top/bottom half
#  meanCorrCfg: optional mean correction from a TrackSmearingTool-style JSON (omegaSmearing
#               mu_data/mu_mc, binned in meanCorrVariable if set); not wanted here since the
#               run-by-run scale replaces it
# Set either to "" to disable it.
anaTrks.parameters["runScaleCfg"] = os.environ['HPSTR_BASE']+"/analysis/data/smearing/fee_scale_smearing_2021_3p74.json"
anaTrks.parameters["meanCorrCfg"] = ""
anaTrks.parameters["meanCorrVariable"] = ""

RegionPath = os.environ['HPSTR_BASE']+"/analysis/selections/feeSmearing/"
anaTrks.parameters["regionDefinitions"] = []

p.sequence = [anaTrks] 

p.input_files = inFilename
p.output_files = [outFilename]

p.printProcess()
