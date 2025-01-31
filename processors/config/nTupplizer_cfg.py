import HpstrConf
import os
import sys

import baseConfig as base
from baseConfig import bfield

#base.parser.add_argument("-N", "--num", type=int, dest="num",
#        help="The JOB number", metavar="num", default=0)

options = base.parser.parse_args()

# Use the input file to set the output file name
inFilename  = options.inFilename[0]
outFilename = '%s_nTup.root' % inFilename[:-5]

print('Input file:  %s' % inFilename)
print('Output file: %s' % outFilename)

p = HpstrConf.Process()

p.run_mode = 1
#p.skip_events = options.skip_events
p.max_events = options.nevents
#p.max_events = 1000

# Library containing processors
p.add_library("libprocessors")

###############################
#          Processors         #
###############################
nTup = HpstrConf.Processor('nTup', 'NTupplizer')
################################
#   Processor Configuration   #
###############################
nTup.parameters["debug"] = 0 
nTup.parameters["trkCollName"] = 'KalmanFullTracks'
nTup.parameters["truthtrkCollName"] = 'Truth_KFTracks'
nTup.parameters["fspCollName"] = 'FinalStateParticles_KF'
nTup.parameters["mcCollName"] = 'MCParticle'
nTup.parameters["sclusCollName"] = 'SiClusters'

nTup.parameters["baselineFile"] = os.environ['HPSTR_BASE']+"/processors/dat/hps_14552_offline_baselines.dat"
nTup.parameters["timeProfiles"] = os.environ['HPSTR_BASE'] + "/processors/dat/hpssvt_014393_database_svt_pulse_shapes_final.dat"
nTup.parameters["outPutCsv"] = 1

p.sequence = [nTup]

p.input_files=[inFilename]
p.output_files = [outFilename]

p.printProcess()
