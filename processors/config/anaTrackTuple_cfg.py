import HpstrConf
import os
import sys
import baseConfig as base

# Use the input file to set the output file name
#inFilename = sys.argv[1].strip()
#outFilename = '%s_anaTrks.root' % inFilename[:-5]

base.parser.add_argument("-r", "--run_number", type=int, dest="run_number",
                                 help="set run number", metavar="run_number", default=-999)
base.parser.add_argument("-TS", "--trackstate", type=str, dest="trackstate",
                                 help="Specify Track State | 'AtECal' oe 'AtTarget'. Default is origin (AtIP) ",
                                  metavar="trackstate", default="")

options = base.parser.parse_args()

inFilename = options.inFilename[0]
outFilename = options.outFilename

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
anaTrks.parameters["trkCollName"] = 'KalmanFullTracks%s'%(options.trackstate)
anaTrks.parameters["run_number"] = options.run_number
if options.year == 2016:
    anaTrks.parameters["beamPosCfg"] = os.environ['HPSTR_BASE']+'/analysis/data/beamspot_positions_2016.json'
else:
        anaTrks.parameters["beamPosCfg"] = ''
# Sequence which the processors will run.
p.sequence = [anaTrks]

p.input_files = [inFilename]
p.output_files = [outFilename]

p.printProcess()
