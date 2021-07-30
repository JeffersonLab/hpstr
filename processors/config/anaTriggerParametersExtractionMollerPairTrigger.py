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

triggerParametersExtractionMollerPairTriggerAna = HpstrConf.Processor('triggerParametersExtractionMollerPairTriggerAna', 'TriggerParametersExtractionMollerPairTriggerAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#RecoHitAna
triggerParametersExtractionMollerPairTriggerAna.parameters["debug"] = 0
triggerParametersExtractionMollerPairTriggerAna.parameters["anaName"] = "triggerParameterExtractionAna"
triggerParametersExtractionMollerPairTriggerAna.parameters["trkColl"] = "KalmanFullTracks"
triggerParametersExtractionMollerPairTriggerAna.parameters["gtpClusColl"] = "RecoEcalClustersGTP"
triggerParametersExtractionMollerPairTriggerAna.parameters["vtxColl"] = "UnconstrainedMollerVertices"
triggerParametersExtractionMollerPairTriggerAna.parameters["partColl"]   = 'ParticlesMollerOnVertices'
triggerParametersExtractionMollerPairTriggerAna.parameters["analysis"] = analysis
triggerParametersExtractionMollerPairTriggerAna.parameters["beamE"] = base.beamE[str(options.year)]
if (options.year == 20211920) : 
    triggerParametersExtractionMollerPairTriggerAna.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/triggerParametersExtraction/triggerParametersExtractionMoller20211920.json'
else:
    triggerParametersExtractionMollerPairTriggerAna.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/triggerParametersExtraction/triggerParametersExtractionMoller.json'    

# Sequence which the processors will run.
p.sequence = [triggerParametersExtractionMollerPairTriggerAna]

p.input_files = infile
p.output_files = [outfile]

if (options.nevents > -1):
    p.max_events = options.nevents

p.printProcess()
