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
anaTrks.parameters["selectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/trackHit/trackHitAna_4p55.json'
anaTrks.parameters["isData"] = options.isData

# FEE (Full Energy Electron) selection
anaTrks.parameters["requireElectron"] = 1     # track charge < 0
anaTrks.parameters["requireCluster"]  = 1     # require an associated full-energy cluster
# FEE trigger NOT required here: the 2019 (4.55 GeV) trigger / TSBank configuration has
# not been verified against the 2021 FEE_Top/FEE_Bot bit convention used by the processor.
# Enabling it before confirming the bits are populated would silently empty the sample.
# Confirm the 2019 TSBank FEE bits, then set requireFeeTrigger = 1.
anaTrks.parameters["requireFeeTrigger"] = 0
# 4.55 GeV beam energy cuts
# FEE cluster energy: same beam-energy fraction as the 3.74 GeV config (~0.80 * beam);
# 0.80 * 4.55 ~= 3.64 GeV
anaTrks.parameters["feeClusterEnergyMin"] = 3.64
# E/p window (off by default: [0, 99]); tighten once the ECal energy scale is calibrated
anaTrks.parameters["eopMin"] = 0.0
anaTrks.parameters["eopMax"] = 99.0
# Cluster-time offset: the 37.3 ns value is the 2021 (PreselectAndCategorize2021) constant.
# It is a DAQ/detector calibration, NOT a beam-energy quantity, so it very likely needs
# retuning for 2019. The window below is applied to the OFFSET-CORRECTED cluster time
# (cluster.getTime() - CalTimeOffset), which should be centered near 0. Run once with
# debug=1 and inspect the [FeeAna][cluTime] dumps to verify/retune the offset for 2019.
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

# Momentum smearing disabled: there is no 2019 (4.55 GeV) smearing tool. This config is
# for looking at the raw FEE tracks. track_smeared branches mirror the unsmeared track.
anaTrks.parameters["doSmearing"] = 0
#anaTrks.parameters["smearingCfg"] = os.environ['HPSTR_BASE']+"/analysis/data/smearing/tool_smearing.json"

RegionPath = os.environ['HPSTR_BASE']+"/analysis/selections/feeSmearing/"
anaTrks.parameters["regionDefinitions"] = []

p.sequence = [anaTrks]

p.input_files = inFilename
p.output_files = [outFilename]

p.printProcess()
