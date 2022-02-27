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

anabeamrotationtargetoffsetting = HpstrConf.Processor('beamrotationtargetoffsettingana', 'BeamRotationTargetOffsettingAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#BeamRotationTargetOffsettingAnaProcessor
anabeamrotationtargetoffsetting.parameters["anaName"] = "beamRotationTargetOffsettingAna"
anabeamrotationtargetoffsetting.parameters["debug"] = 0
anabeamrotationtargetoffsetting.parameters["TSColl"] = "TSBank"
anabeamrotationtargetoffsetting.parameters["trkColl"] = "KalmanFullTracks"
anabeamrotationtargetoffsetting.parameters["vtxColl"] = "UnconstrainedV0Vertices_KF"
anabeamrotationtargetoffsetting.parameters["fspCollRoot"] = "FinalStateParticles_KF"
anabeamrotationtargetoffsetting.parameters["ecalClusColl"] = "RecoEcalClusters"
anabeamrotationtargetoffsetting.parameters["analysis"] = options.analysis

anabeamrotationtargetoffsetting.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/beamRotationTargetOffsetting/beamRotationTargetOffsetting.json'

anabeamrotationtargetoffsetting.parameters["histCfgParticle"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/reco/recoParticle.json'
anabeamrotationtargetoffsetting.parameters["histCfgVertex"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/reco/recoTrackVertex.json'

# Sequence which the processors will run.
p.sequence = [anabeamrotationtargetoffsetting]

p.input_files = infile
p.output_files = [outfile]

if (options.nevents > -1):
    p.max_events = options.nevents

p.printProcess()
