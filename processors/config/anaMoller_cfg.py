import HpstrConf
import sys
import os
import baseConfig as base

#options = base.parser.parse_args()

options = base.parser.parse_args()


print(options)

# Use the input file to set the output file name
infile = options.inFilename
outfile = options.outFilename

analysis = options.analysis



print('Input file: %s' % infile)
print('Output file: %s' % outfile)
print('Analysis : %s' % analysis)

p = HpstrConf.Process()

p.run_mode = 1

# Library containing processors
p.add_library("libprocessors")

###############################
#          Processors         #
###############################

mollerAna = HpstrConf.Processor('mollerAna', 'MollerAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#RecoHitAna
mollerAna.parameters["debug"] = 0
mollerAna.parameters["anaName"] = "mollerAna"
mollerAna.parameters["TSColl"] = "TSBank"
mollerAna.parameters["trkColl"] = "KalmanFullTracks"
#mollerAna.parameters["vtxColl"] = "UnconstrainedMollerVertices"
#mollerAna.parameters["partColl"]   = 'ParticlesMollerOnVertices'
mollerAna.parameters["analysis"] = analysis
mollerAna.parameters["beamE"] = base.beamE[str(options.year)]
mollerAna.parameters["isData"] = options.isData

mollerAna.parameters["vtxSelectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/mollerVertexSelection_2021.json'
mollerAna.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/mollerAnalysis/mollerAnalysis.json'    

# Sequence which the processors will run.
p.sequence = [mollerAna]

p.input_files = infile
p.output_files = [outfile]

if (options.nevents > -1):
    p.max_events = options.nevents

p.printProcess()
