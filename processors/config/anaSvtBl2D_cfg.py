import HpstrConf
import sys
import os
from optparse import OptionParser

parser = OptionParser()

parser.add_option("-i", "--inFile", type="string", dest="inFilename", help="Input filename.", 
        metavar="inFilename", default="")

parser.add_option("-o", "--outFile", type="string", dest="outFilename", help="Output filename.", 
        metavar = "outFilename", default="")

parser.add_option("-t", "--isData", type="int", dest="isData", help="Type of lcio ntuple: 1=data, 0=MC", metavar="isData", default=-1)

(options,args) = parser.parse_args()

# Use the input file to set the output file name
infile = options.inFilename 
if options.outFilename == "":
    outfile = '%s_svtBl2D.root' % infile[:-5]
else:
    outfile = options.outFilename

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
svtblana.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/svt/Svt2DBl.json'

#p.max_events = 1000

# Sequence which the processors will run.
p.sequence = [svtblana]

p.printProcess()
