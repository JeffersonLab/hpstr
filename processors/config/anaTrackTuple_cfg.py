import HpstrConf
import os
import sys

# Use the input file to set the output file name
inFilename  = sys.argv[1].strip()
outFilename = '%s_anaTrks.root' % inFilename[:-5]

print('Input file:  %s' % inFilename)
print('Output file: %s' % outFilename)

p = HpstrConf.Process()

# Library containing processors
p.libraries.append("libprocessors.so")

###############################
#          Processors         #
###############################
anaTrks = HpstrConf.Processor('anaTrks', 'TrackingAnaProcessor')

###############################
#   Processor Configuration   #
###############################
anaTrks.parameters["debug"] = 0 
anaTrks.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/tracking/basicTracking.json'
anaTrks.parameters["trkCollName"] = 'GBLTracks'

# Sequence which the processors will run.
p.sequence = [anaTrks]

p.input_files=[inFilename]
p.output_files = [outFilename]

#p.max_events = 1000

p.printProcess()
