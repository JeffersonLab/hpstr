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

ts = HpstrConf.Processor('tsAna', 'TSAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#RecoHitAna
ts.parameters["debug"] = 0
ts.parameters["anaName"] = "tsAna"
ts.parameters["TSColl"] = "TSBank"
ts.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/ts/ts.json'
ts.parameters["beamE"] = base.beamE[str(options.year)]

# Sequence which the processors will run.
p.sequence = [ts]

p.input_files = infile
p.output_files = [outfile]

p.printProcess()
