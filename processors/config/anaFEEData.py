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

feeDataAna = HpstrConf.Processor('feeDataAna', 'FEEDataAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#RecoHitAna
feeDataAna.parameters["debug"] = 0
feeDataAna.parameters["anaName"] = "feeDataAna"
feeDataAna.parameters["VTPColl"] = "VTPBank"
feeDataAna.parameters["TSColl"] = "TSBank"
feeDataAna.parameters["ecalClusColl"] = "RecoEcalClusters"
feeDataAna.parameters["trkColl"] = "GBLTracks"
feeDataAna.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/feeData/feeData.json'
feeDataAna.parameters["beamE"] = base.beamE[str(options.year)]

# Sequence which the processors will run.
p.sequence = [feeDataAna]

p.input_files = infile
p.output_files = [outfile]

if (options.nevents > -1):
    p.max_events = options.nevents

p.printProcess()
