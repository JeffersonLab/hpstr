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

recoecalana = HpstrConf.Processor('recoecalana', 'RecoEcalAnaProcessor')

recohodoana = HpstrConf.Processor('recohodoana', 'RecoHodoAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#RecoEcalAna
recoecalana.parameters["debug"] = 0
recoecalana.parameters["anaName"] = "recoEcalAna"
recoecalana.parameters["ecalHitColl"] = "RecoEcalHits"
recoecalana.parameters["ecalClusColl"] = "RecoEcalClusters"
recoecalana.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/reco/recoEcal.json'
recoecalana.parameters["analysis"] = options.analysis

#RecoHodoAna
recohodoana.parameters["debug"] = 0
recohodoana.parameters["anaName"] = "recoHodoAna"
recohodoana.parameters["hodoHitColl"] = "RecoHodoHits"
recohodoana.parameters["hodoClusColl"] = "RecoHodoClusters"
recohodoana.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/reco/recoHodo.json'
recohodoana.parameters["analysis"] = options.analysis


# Sequence which the processors will run.
p.sequence = [recoecalana, recohodoana]

p.input_files = infile
p.output_files = [outfile]

p.printProcess()
