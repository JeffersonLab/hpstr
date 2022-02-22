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

recoecalana = HpstrConf.Processor('recoecalana', 'RecoEcalAnaProcessor')

recohodoana = HpstrConf.Processor('recohodoana', 'RecoHodoAnaProcessor')

recotrackvertexana = HpstrConf.Processor('recotrackvertexana', 'RecoTrackVertexAnaProcessor')

recoparticleana = HpstrConf.Processor('recoparticleana', 'RecoParticleAnaProcessor')

###############################
#   Processor Configuration   #
###############################

#TSAna
ts.parameters["debug"] = 0
ts.parameters["anaName"] = "tsAna"
ts.parameters["TSColl"] = "TSBank"
ts.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/ts/ts.json'
ts.parameters["beamE"] = base.beamE[str(options.year)]


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

#RecoHodoAna
recotrackvertexana.parameters["debug"] = 0
recotrackvertexana.parameters["anaName"] = "recoTrackAna"
recotrackvertexana.parameters["trkColl"] = "KalmanFullTracks"
recotrackvertexana.parameters["vtxColl"] = "UnconstrainedV0Vertices_KF"
recotrackvertexana.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/reco/recoTrackVertex.json'
recotrackvertexana.parameters["analysis"] = options.analysis

#RecoParticleAna
recoparticleana.parameters["debug"] = 0
recoparticleana.parameters["anaName"] = "recoParticleAna"
recoparticleana.parameters["fspCollRoot"] = "FinalStateParticles_KF"
recoparticleana.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/reco/recoParticle.json'
recoparticleana.parameters["analysis"] = options.analysis


# Sequence which the processors will run.
p.sequence = [ts, recoecalana, recohodoana, recotrackvertexana, recoparticleana]

p.input_files = infile
p.output_files = [outfile]

if (options.nevents > -1):
    p.max_events = options.nevents

p.printProcess()
