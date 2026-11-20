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
anaTrks.parameters["selectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/trackHit/trackHitAna_1p92.json'
anaTrks.parameters["isData"] = options.isData

# FEE (Full Energy Electron) selection
anaTrks.parameters["requireElectron"] = 1     # track charge < 0
anaTrks.parameters["requireCluster"]  = 1     # require an associated full-energy cluster
# FEE trigger NOT required here: the TSBank FEE_Top/FEE_Bot thresholds (2600-5200 MeV)
# are tuned for the 3.74 GeV beam. Confirm the 1.92 GeV (mollers) run's trigger config
# populates the FEE bits before enabling requireFeeTrigger.
anaTrks.parameters["requireFeeTrigger"] = 1
# 1.92 GeV beam energy cuts
# FEE cluster energy: same beam-energy fraction as 3.74 GeV (3.0 GeV);
# 3.0 * (1.92 / 3.74) ~= 1.54 GeV  (~0.80 * 1.92 GeV beam)
anaTrks.parameters["feeClusterEnergyMin"] = 1.54
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
anaTrks.parameters["vetoExtraClusters"]    = 1
anaTrks.parameters["vetoExtraTracks"]      = 1
anaTrks.parameters["vetoClusterTimeWindow"] = 8.0     # |t_clu - t_FEEclu| (ns)
anaTrks.parameters["vetoTrackTimeWindow"]   = 8.0     # |t_trk - t_FEEclu(corrected)| (ns)
anaTrks.parameters["vetoClusterEnergyMin"]  = 0.5     # only clusters above 0.5 GeV count

#SmearingClosureTest
# Track corrections (data scale corrections, MC smearing), applied to track_smeared:
# one TrackCorrector JSON. doSmearing and the seed stay per-job parameters.
anaTrks.parameters["trackCorrectionsCfg"] = os.environ['HPSTR_BASE']+"/analysis/data/corrections/fee_2021_meancorr.json"
anaTrks.parameters["doSmearing"] = 1 #0 if options.isData else 1
# Scale correction: "" = use omega-space means (default); "tanLambda" = p-space means


RegionPath = os.environ['HPSTR_BASE']+"/analysis/selections/feeSmearing/"
anaTrks.parameters["regionDefinitions"] = []

p.sequence = [anaTrks]

p.input_files = inFilename
p.output_files = [outFilename]

p.printProcess()
