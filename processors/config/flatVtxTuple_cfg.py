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
p.skip_events = options.skip_events
p.max_events = options.nevents

#p.max_events = 1000

# Library containing processors
p.add_library("libprocessors")

###############################
#          Processors         #
###############################

vtxana = HpstrConf.Processor('vtxana', 'FlatVertexProcessor')

###############################
#   Processor Configuration   #
###############################
#Vertex Analysis
vtxana.parameters["debug"] = 0
vtxana.parameters["anaName"] = "vtxana"
vtxana.parameters["tsColl"] = "TSBank"
vtxana.parameters["trkColl"] = "KalmanFullTracks"
vtxana.parameters["hitColl"] = "SiClustersOnTrack"
vtxana.parameters["vtxColl"] = "UnconstrainedV0Vertices_KF"
vtxana.parameters["mcColl"] = "MCParticle"
vtxana.parameters["analysis"] = "vertex"
vtxana.parameters["vtxSelectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/empty.json'
vtxana.parameters["beamE"] = base.beamE[str(options.year)]
vtxana.parameters["isData"] = options.isData
vtxana.parameters["isRadPDG"] = 622
vtxana.parameters["makeFlatTuple"] = True

CalTimeOffset = -999
eleTrackTimeBias = -999
posTrackTimeBias = -999

if (options.isData == 1):
    #CalTimeOffset = 56.
    CalTimeOffset = 37.4
#    eleTrackTimeBias = 0.46
#    posTrackTimeBias = 0.46
    eleTrackTimeBias = 0.0
    posTrackTimeBias = 0.0
    print("Running on data file: Setting CalTimeOffset %d" % CalTimeOffset)

elif (options.isData == 0):
    #CalTimeOffset = 43.
    CalTimeOffset = 24.
    #eleTrackTimeBias = 28.9
    #posTrackTimeBias = 28.9
    eleTrackTimeBias = 35.1 #55 for no spacing
    posTrackTimeBias = 35.1 #55 for no spacing
    print("Running on MC file: Setting CalTimeOffset %d" % CalTimeOffset)
else:
    print("Specify which type of ntuple you are running on: -t 1 [for Data] / -t 0 [for MC]")


vtxana.parameters["CalTimeOffset"] = CalTimeOffset
vtxana.parameters["eleTrackTimeBias"] = eleTrackTimeBias
vtxana.parameters["posTrackTimeBias"] = posTrackTimeBias


#Region definitions

RegionPath = os.environ['HPSTR_BASE']+"/analysis/selections/"
if (options.year == 2019):
    vtxana.parameters["regionDefinitions"] = [RegionPath+'Tight_2019.json', RegionPath+'Tight_pTop_2019.json', RegionPath+'Tight_pBot_2019.json']
if (options.year == 2021):
    vtxana.parameters["regionDefinitions"] = [RegionPath+'empty.json', RegionPath+'preselectionCuts/sanity_cuts.json']

# Sequence which the processors will run.
p.sequence = [vtxana]

p.input_files = infile
p.output_files = [outfile]

p.printProcess()
