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
mollerAna.parameters["vtxColl"] = "UnconstrainedMollerVertices_KF"
mollerAna.parameters["partColl"]   = 'ParticlesMollerOnVertices_KF'
mollerAna.parameters["analysis"] = analysis
mollerAna.parameters["beamE"] = base.beamE[str(options.year)]
mollerAna.parameters["isData"] = options.isData

if options.year == 2021:
    mollerAna.parameters["trackSelectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/2021/moller/mollerTrackSelection_2021.json'
    mollerAna.parameters["vertexSelectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/2021/moller/mollerVertexSelection_2021.json'
    mollerAna.parameters["histTrackCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/moller/mollerTrackAnalysis_2021.json'
    mollerAna.parameters["histVertexCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/moller/mollerVertexAnalysis_2021.json'  
elif options.year == 20211920:
    mollerAna.parameters["trackSelectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/2021/moller/mollerTrackSelection_20211920.json'
    mollerAna.parameters["vertexSelectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/2021/moller/mollerVertexSelection_20211920.json'
    mollerAna.parameters["histTrackCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/moller/mollerTrackAnalysis_20211920.json'
    mollerAna.parameters["histVertexCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/moller/mollerVertexAnalysis_20211920.json'     

# Sequence which the processors will run.
p.sequence = [mollerAna]

p.input_files = infile
p.output_files = [outfile]

if (options.nevents > -1):
    p.max_events = options.nevents

p.printProcess()
