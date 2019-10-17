import HpstrConf
import sys,os

# Use the input file to set the output file name
infilename = sys.argv[1].strip()
outfilename = sys.argv[2].strip()

print 'LCIO file: %s' % infilename
print 'Root file: %s' % outfilename

p = HpstrConf.Process()

# Library containing processors
p.libraries.append("libprocessors.so")

#Processors

clusters = HpstrConf.Processor('clusters','ClusterOnTrackAnaProcessor')
clusters.parameters["BaselineFits"] = "/nfs/hps3/svtTests/jlabSystem/baselines/fits/"
clusters.parameters["BaselineRun"]  = "010705"

p.sequence = [clusters]

p.input_files    = [infilename]

p.output_files  = [outfilename]

#p.max_events   = 1000
p.printProcess()

