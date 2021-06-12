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

feeMCAna = HpstrConf.Processor('feeMCAna', 'FEEMCAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#RecoHitAna
feeMCAna.parameters["debug"] = 0
feeMCAna.parameters["anaName"] = "feeMCAna"
feeMCAna.parameters["gtpClusColl"] = "RecoEcalClustersGTP"
feeMCAna.parameters["ecalClusColl"] = "RecoEcalClusters"
feeMCAna.parameters["ecalClusColl"] = "RecoEcalClusters"
feeMCAna.parameters["trkColl"] = "GBLTracks"
feeMCAna.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/feeData/feeData.json'
feeMCAna.parameters["beamE"] = base.beamE[str(options.year)]

# Sequence which the processors will run.
p.sequence = [feeMCAna]

p.input_files = infile
p.output_files = [outfile]

if (options.nevents > -1):
    p.max_events = options.nevents

p.printProcess()
