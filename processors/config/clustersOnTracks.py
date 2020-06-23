import HpstrConf
import sys,os

# Use the input file to set the output file name
infilename = sys.argv[1].strip()
outfilename = sys.argv[2].strip()

print('Input file:  %s' % infilename)
print('Output file: %s' % outfilename)

p = HpstrConf.Process()

p.run_mode   = 1
#p.max_events   = 1000

# Library containing processors
p.add_library("libprocessors")

#Processors
clusters = HpstrConf.Processor('clusters','ClusterOnTrackAnaProcessor')

#Processor Configurations
clusters.parameters["debug"] = 0
clusters.parameters["anaName"] = 'anaClusOnTrk'
clusters.parameters["trkColl"] = 'GBLTracks'
clusters.parameters["BaselineFits"] = "/nfs/hps3/svtTests/jlabSystem/baselines/fits/"
clusters.parameters["BaselineRun"]  = "010705"

p.sequence = [clusters]

p.input_files    = [infilename]

p.output_files  = [outfilename]

p.printProcess()

