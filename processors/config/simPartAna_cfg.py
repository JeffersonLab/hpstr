import HpstrConf
import sys
import os
import baseConfig as base


base.parser.add_argument("-w", "--tracking", type=str, dest="tracking",
                         help="Which tracking to use to make plots", metavar="tracking", default="KF")
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
p.skip_events = options.skip_events
p.max_events = options.nevents

#p.max_events = 1000

# Library containing processors
p.add_library("libprocessors")

###############################
#          Processors         #
###############################

sim_part_ana = HpstrConf.Processor('sim_part', 'SimPartProcessor')

###############################
#   Processor Configuration   #
###############################
#RecoHitAna
sim_part_ana.parameters["debug"] = 0
sim_part_ana.parameters["anaName"] = "sim_part"
sim_part_ana.parameters["MCParticleColl"] = "MCParticle"
sim_part_ana.parameters["MCTrackerHitColl"] = "TrackerSimHits"
sim_part_ana.parameters["MCEcalHitColl"] = "EcalSimHits"
sim_part_ana.parameters["RecoTrackColl"] = "KalmanFullTracks"
sim_part_ana.parameters["RecoTrackerClusterColl"] = "SiClustersOnTrack"
sim_part_ana.parameters["RecoEcalClusterColl"] = "RecoEcalClusters"
sim_part_ana.parameters["analysis"] = analysis
sim_part_ana.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/mc/simPart.json'

# Sequence which the processors will run.
p.sequence = [sim_part_ana]

p.input_files = infile
p.output_files = [outfile]

p.printProcess()
