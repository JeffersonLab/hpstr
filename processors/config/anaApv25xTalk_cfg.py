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

apvana = HpstrConf.Processor('apvana', 'Apv25RoXtalkAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#RecoHitAna
apvana.parameters["debug"] = 0
apvana.parameters["anaName"] = "apvAna"
apvana.parameters["rawHitColl"] = "SVTRawTrackerHits"
apvana.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/mc/basicMC.json'

# Sequence which the processors will run.
p.sequence = [apvana]

p.input_files = infile
p.output_files = [outfile]

p.printProcess()
