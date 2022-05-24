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
anabeamrotationtargetoffsetting.parameters["histCfgVertex"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/tracking/vtxAnalysis_2019.json'

anabeamrotationtargetoffsetting.parameters["vtxSelectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/2019/vertexSelection.json'

anabeamrotationtargetoffsetting.parameters["beamE"] = base.beamE[str(options.year)]
anabeamrotationtargetoffsetting.parameters["isData"] = options.isData

CalTimeOffset=-999
if (options.isData==1):
    CalTimeOffset=39.
    print("Running on data file: Setting CalTimeOffset %d"  % CalTimeOffset)
    
elif (options.isData==0):
    CalTimeOffset=39.
    print("Running on MC file: Setting CalTimeOffset %d"  % CalTimeOffset)
else:
    print("Specify which type of ntuple you are running on: -t 1 [for Data] / -t 0 [for MC]")


anabeamrotationtargetoffsetting.parameters["CalTimeOffset"]=CalTimeOffset

# Sequence which the processors will run.
p.sequence = [anabeamrotationtargetoffsetting]

p.input_files = infile
p.output_files = [outfile]

if (options.nevents > -1):
    p.max_events = options.nevents

p.printProcess()
