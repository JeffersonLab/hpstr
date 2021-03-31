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

ecalTimingAna = HpstrConf.Processor('ecalTimingAna', 'EcalTimingAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#RecoHitAna
ecalTimingAna.parameters["debug"] = 0
ecalTimingAna.parameters["anaName"] = "ecalTimingAna"
ecalTimingAna.parameters["VTPColl"] = "VTPBank"
ecalTimingAna.parameters["TSColl"] = "TSBank"
ecalTimingAna.parameters["ecalClusColl"] = "RecoEcalClusters"
ecalTimingAna.parameters["trkColl"] = "GBLTracks"
ecalTimingAna.parameters["vtxColl"] = "TargetConstrainedV0Vertices"
ecalTimingAna.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/ecalTiming/ecalTiming.json'
ecalTimingAna.parameters["beamE"] = base.beamE[str(options.year)]

# Sequence which the processors will run.
p.sequence = [ecalTimingAna]

p.input_files = infile
p.output_files = [outfile]

p.printProcess()
