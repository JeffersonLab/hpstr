import HpstrConf
import sys
import os

# Use the input file to set the output file name
infile = sys.argv[1].strip()
outfile = '%s_anaReco.root' % infile[:-5]

print 'Input file: %s' % infile
print 'Output file: %s' % outfile

p = HpstrConf.Process()

# Library containing processors
p.libraries.append("libprocessors.so")

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

p.input_files=[infile]
p.output_files = [outfile]

#p.max_events = 1000

p.printProcess()
