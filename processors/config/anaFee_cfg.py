#
# anaFee_cfg.py
#
# Configuration for the FeeAnaProcessor: selects Full Energy Electron (FEE) events
# off the FinalStateParticle collection, where each track is already associated to
# its ECal cluster at the LCIO stage (no manual track-cluster matching).
#
# Example:
#   hpstr anaFee_cfg.py -i <ntuple.root> -o fee.root -t 1 -s <fileNumber>
#
import HpstrConf
import os
import sys
import baseConfig as base

options = base.parser.parse_args()

# Use the input file to set the output file name
inFilename   = options.inFilename
outFilename  = options.outFilename

# The seed is the file number (used by the smearing tool).
smearingSeed = options.seed

print('Input file:  %s' % inFilename)
print('Output file: %s' % outFilename)

p = HpstrConf.Process()

p.run_mode    = 1
p.skip_events = options.skip_events
p.max_events  = options.nevents

p.add_library("libprocessors")

###############################
#   FEE analysis processor    #
###############################
anaFee = HpstrConf.Processor('anaFee', 'FeeAnaProcessor')

anaFee.parameters["debug"]  = options.debug
anaFee.parameters["seed"]   = smearingSeed
anaFee.parameters["isData"] = options.isData

# Track collection name -- used only for histogram-directory and smearing-tool naming.
anaFee.parameters["trkCollName"] = 'KalmanFullTracks'
# FinalStateParticle collection that the FEE selection actually loops over. Each
# particle carries its track and (when matched) its ECal cluster.
anaFee.parameters["fspCollName"] = 'FinalStateParticles_KF'

# Histogram + track-selection configuration
anaFee.parameters["histCfg"]       = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/fee_smearing/feeSmearing_2021.json'
anaFee.parameters["selectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/trackHit/trackHitAna.json'

###############################
#   FEE selection            #
###############################
anaFee.parameters["requireElectron"]    = 1     # require track charge < 0 (electron)
anaFee.parameters["requireCluster"]      = 1     # require an associated full-energy cluster
anaFee.parameters["feeClusterEnergyMin"] = 2.5   # GeV; ~0.68 * 3.7 GeV beam

# Cluster-time offset: the raw ECal cluster time has a fixed offset relative to the
# (already-calibrated) track time. Measured from the prompt FEE peak in pass5_v9 data:
#   mean 38.6 ns, sigma 1.5 ns.  The window below is applied to the OFFSET-CORRECTED
# time (cluster.getTime() - CalTimeOffset), which is centered near 0.
# NOTE: CalTimeOffsetMC must be re-measured on MC (placeholder below).
if options.isData:
    anaFee.parameters["CalTimeOffset"]   = 38.6
else:
    anaFee.parameters["CalTimeOffsetMC"] = 38.6   # TODO: measure on MC

# Corrected cluster-time window (~ +/-4 sigma around the prompt peak)
anaFee.parameters["clusterTimeMin"]   = -6.0
anaFee.parameters["clusterTimeMax"]   =  6.0
anaFee.parameters["clusterTimeMinMC"] = -6.0
anaFee.parameters["clusterTimeMaxMC"] =  6.0
anaFee.parameters["mcTimeOffset"]     = 5.0
# E/p window (off by default: [0, 99]); tighten once the ECal energy scale is calibrated.
anaFee.parameters["eopMin"] = 0.0
anaFee.parameters["eopMax"] = 99.0

###############################
#   Momentum smearing        #
###############################
anaFee.parameters["smearingCfg"]    = os.environ['HPSTR_BASE']+"/analysis/data/smearing/tool_smearing.json"
anaFee.parameters["doSmearing"]     = 1
anaFee.parameters["smearingFactor"] = 1.0
anaFee.parameters["smearOmega"]     = 1
# Smearing lookup variable: "flat", "nHits", "tanLambda", "phi0" ("" = JSON default)
anaFee.parameters["smearingVariable"] = "phi0"
# Scale correction: "" = use omega-space means (default); "tanLambda" = p-space means
anaFee.parameters["scaleCorrVariable"] = ""

# Optional per-region selections (empty = none)
anaFee.parameters["regionDefinitions"] = []

p.sequence = [anaFee]

p.input_files  = inFilename
p.output_files = [outFilename]

p.printProcess()
