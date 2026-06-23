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
anaTrks.parameters["feeClusterEnergyMin"] = 2.5  # ~0.68 * 3.7 GeV beam
# E/p window (off by default: [0, 99]); tighten once the ECal energy scale is calibrated
anaTrks.parameters["eopMin"] = 0.0
anaTrks.parameters["eopMax"] = 99.0

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

RegionPath = os.environ['HPSTR_BASE']+"/analysis/selections/feeSmearing/"
anaTrks.parameters["regionDefinitions"] = []

p.sequence = [anaTrks] 

p.input_files = inFilename
p.output_files = [outFilename]

p.printProcess()
