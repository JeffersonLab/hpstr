import HpstrConf
import sys
import os
#import baseConfig as base

#base.parser.add_argument('-b', '--baselines', type=str, dest="baselines",default="",
#        help="input offline baseline analysis root file")

# Use the input file to set the output file name
infilename = sys.argv[1].strip()
outfilename = sys.argv[2].strip()

#options = base.parser.parse_args()
#infilename = options.inFilename
#outfilename = options.outFilename

print('Input file:  %s' % infilename)
print('Output file: %s' % outfilename)

p = HpstrConf.Process()

p.run_mode = 1
p.skip_events = options.skip_events
p.max_events = options.nevents

#p.max_events   = 1000

# Library containing processors
p.add_library("libprocessors")

#Processors
clusters = HpstrConf.Processor('clusters', 'ClusterOnTrackAnaProcessor')

#Processor Configurations
clusters.parameters["debug"] = 1
clusters.parameters["anaName"] = 'anaClusOnTrk'
clusters.parameters["trkColl"] = 'KalmanFullTracks'
#clusters.parameters["BaselineFits"] = "/home/alic/HPS/projects/baselines/jlab/clusters_on_track/"
clusters.parameters["BaselineFits"] = "/home/alic/HPS/projects/baselines/jlab/clusters_on_track/hps_14552_offline_analysis.root"
#clusters.parameters["BaselineFits"] = options.baselines
clusters.parameters["BaselineRun"] = "010705"

p.sequence = [clusters]

p.input_files = [infilename]

p.output_files = [outfilename]

p.printProcess()
