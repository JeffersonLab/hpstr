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

recoana = HpstrConf.Processor('recoana', 'RecoHitAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#RecoHitAna
recoana.parameters["debug"] = 0
recoana.parameters["anaName"] = "recoHitAna"
recoana.parameters["trkColl"] = "GBLTracks"
recoana.parameters["trkrHitColl"] = "RotatedHelicalTrackHits"
recoana.parameters["ecalHitColl"] = "RecoEcalHits"
recoana.parameters["ecalClusColl"] = "RecoEcalClusters"
recoana.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/reco/basicRecoHit.json'

# Sequence which the processors will run.
p.sequence = [recoana]

p.input_files = infile
p.output_files = [outfile]

p.printProcess()
