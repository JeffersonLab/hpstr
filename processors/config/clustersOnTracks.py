import HpstrConf
import sys,os
import baseConfig as base

base.parser.add_argument('-b', '--baselineFits', type=str, dest="baselineFits",default="/home/alic/HPS/projects/baselines/jlab/clusters_on_track/",
        help="Provide full path to charge_cluster_baselines.root")
base.parser.add_argument('-r', '--run', type=str, dest="run",default="0000",
        help="run number")

options = base.parser.parse_args()

# Use the input file to set the output file name
#infilename = sys.argv[1].strip()
infilename = options.inFilename
outfilename = options.outFilename


print('Input file:  %s' % infilename)
print('Output file: %s' % outfilename)

p = HpstrConf.Process()

p.run_mode   = 1
#p.max_events   = 1000

# Library containing processors
#p.add_library("libprocessors")
# Library containing processors
p.libraries.append("libprocessors.so")

#Processors
clusters = HpstrConf.Processor('clusters','ClusterOnTrackAnaProcessor')

#Processor Configurations
clusters.parameters["debug"] = 1
clusters.parameters["anaName"] = 'anaClusOnTrk'
clusters.parameters["trkColl"] = 'KalmanFullTracks'
clusters.parameters["BaselineFits"] = options.baselineFits
clusters.parameters["BaselineRun"]  = options.run

p.sequence = [clusters]

p.input_files    = [infilename]

p.output_files  = [outfilename]

p.printProcess()

