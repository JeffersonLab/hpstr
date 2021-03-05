import HpstrConf
import sys
import os
import baseConfig as base

options = base.parser.parse_args()

# Use the input file to set the output file name
infile = options.inFilename
outfile = options.outFilename

analysis = options.analysis



print('Input file: %s' % infile)
print('Output file: %s' % outfile)
print('Analysis : %s' % analysis)

p = HpstrConf.Process()

p.run_mode = 1
#p.max_events = 1000

# Library containing processors
p.add_library("libprocessors")

###############################
#          Processors         #
###############################

mcana = HpstrConf.Processor('mcana', 'MCAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#RecoHitAna
mcana.parameters["debug"] = 0
mcana.parameters["anaName"] = "mcAna"
mcana.parameters["partColl"] = "MCParticle"
mcana.parameters["trkrHitColl"] = "TrackerHit"
mcana.parameters["ecalHitColl"] = "CalHit"
mcana.parameters["analysis"] = analysis
mcana.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/mc/basicMC.json'

# Sequence which the processors will run.
p.sequence = [mcana]

p.input_files = infile
p.output_files = [outfile]

p.printProcess()
