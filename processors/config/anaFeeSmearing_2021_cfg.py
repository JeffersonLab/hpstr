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

anaTrks = HpstrConf.Processor('anaTrks', 'TrackingAnaProcessor')
anaTrks.parameters["debug"] = 0
anaTrks.parameters["seed"] = smearingSeed
anaTrks.parameters["trkCollName"] = 'KalmanFullTracks'
anaTrks.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/fee_smearing/feeSmearing_2021.json'
anaTrks.parameters["selectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/trackHit/trackHitAna.json'
anaTrks.parameters["isData"] = options.isData

#SmearingClosureTest
anaTrks.parameters["smearingCfg"] = os.environ['HPSTR_BASE']+"/analysis/data/smearing/tool_smearing.json"
anaTrks.parameters["doSmearing"] = 1 #0 if options.isData else 1
anaTrks.parameters["smearingFactor"] = 1.0
anaTrks.parameters["smearOmega"] = 1
#anaTrks.parameters["smearingVariable"] = "tanLambda"
anaTrks.parameters["smearingVariable"] = "phi0"
#anaTrks.parameters["smearingVariable"] = "flat"

RegionPath = os.environ['HPSTR_BASE']+"/analysis/selections/feeSmearing/"
anaTrks.parameters["regionDefinitions"] = []

p.sequence = [anaTrks] 

p.input_files = inFilename
p.output_files = [outFilename]

p.printProcess()
