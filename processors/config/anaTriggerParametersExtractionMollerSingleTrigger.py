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

triggerParametersExtractionMollerSingleTriggerAna = HpstrConf.Processor('triggerParametersExtractionMollerAna', 'TriggerParametersExtractionMollerSingleTriggerAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#RecoHitAna
triggerParametersExtractionMollerSingleTriggerAna.parameters["debug"] = 0
triggerParametersExtractionMollerSingleTriggerAna.parameters["anaName"] = "triggerParameterExtractionAna"
triggerParametersExtractionMollerSingleTriggerAna.parameters["trkColl"] = "KalmanFullTracks"
triggerParametersExtractionMollerSingleTriggerAna.parameters["gtpClusColl"] = "RecoEcalClustersGTP"
triggerParametersExtractionMollerSingleTriggerAna.parameters["vtxColl"] = "UnconstrainedMollerVertices"
triggerParametersExtractionMollerSingleTriggerAna.parameters["partColl"]   = 'ParticlesMollerOnVertices'
triggerParametersExtractionMollerSingleTriggerAna.parameters["analysis"] = analysis
triggerParametersExtractionMollerSingleTriggerAna.parameters["beamE"] = base.beamE[str(options.year)]
if (options.year == 20211920) : 
    triggerParametersExtractionMollerSingleTriggerAna.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/triggerParametersExtraction/triggerParametersExtractionMoller20211920.json'
else:
    triggerParametersExtractionMollerSingleTriggerAna.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/triggerParametersExtraction/triggerParametersExtractionMoller.json'    

# Sequence which the processors will run.
p.sequence = [triggerParametersExtractionMollerSingleTriggerAna]

p.input_files = infile
p.output_files = [outfile]

if (options.nevents > -1):
    p.max_events = options.nevents

p.printProcess()
