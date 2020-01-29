import HpstrConf
import sys
import os
import baseConfig

(options,args) = baseConfig.parser.parse_args()


# Use the input file to set the output file name
infile = options.inFilename
outfile = options.outFilename

print 'Input file: %s' % infile
print 'Output file: %s' % outfile

p = HpstrConf.Process()

p.run_mode = 1
#p.max_events = 1000

# Library containing processors
p.add_library("libprocessors")

###############################
#          Processors         #
###############################

recoana = HpstrConf.Processor('vtxana', 'VertexAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#RecoHitAna
recoana.parameters["debug"] = 1
recoana.parameters["anaName"] = "vtxana"
recoana.parameters["trkColl"] = "GBLTracks"
recoana.parameters["vtxColl"] = "UnconstrainedV0Vertices"
recoana.parameters["vtxSelectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/vertexSelection.json'
#recoana.parameters["vtxSelectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/vtxSelection_noLyReq.json'
recoana.parameters["histoCfg"] = os.environ['HPSTR_BASE']+"/analysis/plotconfigs/tracking/vtxAnalysis.json"
recoana.parameters["beamE"] = 2.3
recoana.parameters["isData"] = options.isData
CalTimeOffset=-999

if (options.isData==1):
    CalTimeOffset=56.
    print "Running on data file: Setting CalTimeOffset %d"  % CalTimeOffset
    
elif (options.isData==0):
    CalTimeOffset=43.
    print "Running on MC file: Setting CalTimeOffset %d"  % CalTimeOffset
else:
    print "Specify which type of ntuple you are running on: -t 1 [for Data] / -t 0 [for MC]"


recoana.parameters["CalTimeOffset"]=CalTimeOffset

#Region definitions

RegionPath=os.environ['HPSTR_BASE']+"/analysis/selections/"
recoana.parameters["regionDefinitions"] = [RegionPath+'Tight.json']
#    
#    RegionPath+'ESumCR.json',
#    RegionPath+'TightNoSharedL0.json',
#    RegionPath+'TightNoShared.json',



# Sequence which the processors will run.
p.sequence = [recoana]

p.input_files=[infile]
p.output_files = [outfile]

p.printProcess()


