import HpstrConf
import os
import sys

# Use the input file to set the output file name
inFilename = sys.argv[1].strip()
outFilename = '%s_anaTrks.root' % inFilename[:-5]

print('Input file:  %s' % inFilename)
print('Output file: %s' % outFilename)

p = HpstrConf.Process()

p.run_mode = 1
p.skip_events = options.skip_events
p.max_events = options.nevents
#p.max_events = 1000

# Library containing processors
p.add_library("libprocessors")

###############################
#          Processors         #
###############################
anaTrks = HpstrConf.Processor('anaTrks', 'TrackingAnaProcessor')

###############################
#   Processor Configuration   #
###############################
anaTrks.parameters["debug"] = 0
anaTrks.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/tracking/basicTracking.json'
anaTrks.parameters["trkCollName"] = 'KalmanFullTracks'

# Sequence which the processors will run.
p.sequence = [anaTrks]

p.input_files = [inFilename]
p.output_files = [outFilename]

p.printProcess()
