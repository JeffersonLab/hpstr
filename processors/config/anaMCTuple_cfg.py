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

mcana = HpstrConf.Processor('mcana', 'MCAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#RecoHitAna
mcana.parameters["debug"] = 1
mcana.parameters["anaName"] = "mcAna"
mcana.parameters["partColl"] = "MCParticle"
mcana.parameters["trkrHitColl"] = "TrackerHits"
mcana.parameters["ecalHitColl"] = "EcalHits"
mcana.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/mc/findableMC.json'

# Define regions
RegionPath=os.environ['HPSTR_BASE']+"/analysis/selections/mc"
print(RegionPath+'/mcFindablePair.json')
mcana.parameters["regionDefinitions"] = [RegionPath+'/mcNoCuts.json',RegionPath+'/mcFindablePair.json']
#mcana.parameters["regionDefinitions"] = ['./mcFindablePair.json']

# Sequence which the processors will run.
p.sequence = [mcana]

p.input_files = infile
p.output_files = [outfile]

p.printProcess()
