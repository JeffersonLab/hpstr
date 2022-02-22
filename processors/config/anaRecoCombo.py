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

recocomboana = HpstrConf.Processor('recocomboana', 'RecoComboAnaProcessor')

###############################
#   Processor Configuration   #
###############################

#TSAna
ts.parameters["debug"] = 0
ts.parameters["anaName"] = "tsAna"
ts.parameters["TSColl"] = "TSBank"
ts.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/ts/ts.json'
ts.parameters["beamE"] = base.beamE[str(options.year)]


#RecoComboAna
recocomboana.parameters["debug"] = 0
recocomboana.parameters["anaName"] = "recoComboAna"
recocomboana.parameters["ecalHitColl"] = "RecoEcalHits"
recocomboana.parameters["ecalClusColl"] = "RecoEcalClusters"

recocomboana.parameters["hodoHitColl"] = "RecoHodoHits"
recocomboana.parameters["hodoClusColl"] = "RecoHodoClusters"

recocomboana.parameters["trkColl"] = "KalmanFullTracks"
recocomboana.parameters["vtxColl"] = "UnconstrainedV0Vertices_KF"

recocomboana.parameters["fspCollRoot"] = "FinalStateParticles_KF"

recocomboana.parameters["histCfgEcal"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/reco/recoEcal.json'
recocomboana.parameters["histCfgHodo"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/reco/recoHodo.json'
recocomboana.parameters["histCfgTrack"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/reco/recoTrack.json'
recocomboana.parameters["histCfgParticle"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/reco/recoParticle.json'
recocomboana.parameters["analysis"] = options.analysis

# Sequence which the processors will run.
p.sequence = [ts, recocomboana]

p.input_files = infile
p.output_files = [outfile]

if (options.nevents > -1):
    p.max_events = options.nevents

p.printProcess()
