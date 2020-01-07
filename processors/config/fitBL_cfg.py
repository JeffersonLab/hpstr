import HpstrConf
import sys

from optparse import OptionParser

parser = OptionParser()
parser.add_option("-i", "--inFile", type="string", dest="inFilename",
        help="Input filename.", metavar="inFilename", default="")
parser.add_option("-d", "--outDir", type="string", dest="outDir",
        help="Specify the output directory.", metavar="outDir", default=".")
parser.add_option("-r", "--run", type="string", dest="runNum",
        help="Specify the Run File.", metavar="runNum", default="")
(options, args) = parser.parse_args()

# Use the input file to set the output file name
histo_file = options.inFilename
fit_file = '%s/fitBL_Run_%s'%(options.outDir, options.runNum)

p = HpstrConf.Process()

p.run_mode = 2
#p.max_events = 1000

# Library containing processors
p.libraries.append("libprocessors.so")

###############################
#          Processors         #
###############################

fitBL = HpstrConf.Processor('fitBL', 'SvtBlFitHistoProcessor')

###############################
#   Processor Configuration   #
###############################
#MCParticles
# Sequence which the processors will run.
p.sequence = [fitBL]

p.input_files=[histo_file]
p.output_files = [fit_file]

p.printProcess()
