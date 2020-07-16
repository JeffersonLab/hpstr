import HpstrConf
import sys
import os
import baseConfig as base

options = base.parser.parse_args()


# Use the input file to set the output file name
infile = options.inFilename
outfile = options.outFilename

print('Input file: %s' % infile)
print('Output file: %s' % outfile)

p = HpstrConf.Process()

p.run_mode = 1
#p.max_events = 1000

# Library containing processors
p.add_library("libprocessors")

###############################
#          Processors         #
###############################
trkana    = HpstrConf.Processor('trkana','TrackingAnaProcessor')
trkgblana = HpstrConf.Processor('trkgblana','TrackingAnaProcessor')

trkana.parameters["debug"]  = 0;
trkana.parameters["trkCollName"] = "KalmanFullTracks"
trkana.parameters["histCfg"] = os.environ['HPSTR_BASE'] + '/analysis/plotconfigs/tracking/basicTracking.json'
trkana.parameters["doTruth"] = 1
trkana.parameters["truthHistCfg"] = os.environ['HPSTR_BASE'] + '/analysis/plotconfigs/tracking/truthTrackComparison.json'


trkgblana.parameters["debug"]        = 0;
trkgblana.parameters["trkCollName"]  = "GBLRefittedTracks"
trkgblana.parameters["histCfg"]      = os.environ['HPSTR_BASE'] + '/analysis/plotconfigs/tracking/basicTracking.json'
trkgblana.parameters["doTruth"]      = 1
trkgblana.parameters["truthHistCfg"] = os.environ['HPSTR_BASE'] + '/analysis/plotconfigs/tracking/truthTrackComparison.json'

p.sequence = [trkana,trkgblana]

p.input_files = infile
p.output_files = [outfile]

p.printProcess()
