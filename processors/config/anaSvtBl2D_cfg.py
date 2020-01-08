import HpstrConf
import sys

# Use the input file to set the output file name
infile = sys.argv[1].strip()
outfile = '%s_svtBl2D.root' % infile[:-5]

print 'Input file: %s' % infile
print 'Output file: %s' % outfile

p = HpstrConf.Process()
p.run_mode = 1

#Set files to process
p.input_files=[infile]
p.output_files = [outfile]

# Library containing processors
p.libraries.append("libprocessors.so")

###############################
#          Processors         #
###############################

svtblana = HpstrConf.Processor('svtblana', 'SvtBl2DAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#SvtBl2D
svtblana.parameters["debug"] = 1
svtblana.parameters["removeBaseline"] = 0
svtblana.parameters["baselineFits"] = "filename"
svtblana.parameters["runNumber"] = 135
svtblana.parameters["rawSvtHitsColl"] = "SVTRawTrackerHits"

#p.max_events = 1000

# Sequence which the processors will run.
p.sequence = [svtblana]

p.printProcess()
