import HpstrConf
import os
import sys
import baseConfig as base

options = base.parser.parse_args()

# Use the input file to set the output file name
inFilename = options.inFilename
outFilename = options.outFilename

print('Input file:  %s' % inFilename)
print('Output file: %s' % outFilename)

p = HpstrConf.Process()

p.run_mode = 1
p.skip_events = options.skip_events
p.max_events = options.nevents


p.add_library("libprocessors")

#anaTrks = HpstrConf.Processor('anaTrks', 'TrackHitAnaProcessor')
anaTrks = HpstrConf.Processor('anaTrks', 'TrackingAnaProcessor')
anaTrks.parameters["debug"] = 0
anaTrks.parameters["trkCollName"] = 'KalmanFullTracks'
anaTrks.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/fee_smearing/feeSmearing.json'
anaTrks.parameters["selectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/trackHit/trackHitAna.json'
anaTrks.parameters["isData"] = options.isData

#SmearingClosureTest
anaTrks.parameters["pSmearingFile"] =  os.environ['HPSTR_BASE']+"/utils/data/smearingFile_2016_all_12112023.root"

RegionPath = os.environ['HPSTR_BASE']+"/analysis/selections/feeSmearing/"
anaTrks.parameters["regionDefinitions"] = []

p.sequence = [anaTrks] 

p.input_files = inFilename
p.output_files = [outFilename]

p.printProcess()
