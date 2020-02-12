import HpstrConf
import sys
import os
from optparse import OptionParser

def timeSample_callback(options, opt, value, parser):
    setattr(parser.values, options.dest, value.split(','))

parser = OptionParser()
parser.add_option("-i", "--inFile", type="string", dest="inFilename",
        help="Input filename.", metavar="inFilename", default="")
parser.add_option("-c", "--nhitsFitCut", type="int", dest="nhitsFitCut",
        help="set the min number of hits required for Gauss Fit of channels", metavar="nhitsFitCut", default="0")
parser.add_option("-d", "--outDir", type="string", dest="outDir",
        help="Specify the output directory.", metavar="outDir", default=".")
parser.add_option("-n", "--nPoints", type="int", dest="nPoints",
        help="Select number of points for second derivative.", metavar="nPoints", default="1")
parser.add_option("-b", "--rebin", type="int", dest="rebin",
        help="rebin factor.", metavar="rebin", default="1")
parser.add_option('-s', '--hybrid', type='string', dest="hybrid", help="Enter baseline<#><hybrid_name>", action='callback',
        callback=timeSample_callback)
(options, args) = parser.parse_args()

# Use the input file to set the output file name
histo_file = options.inFilename
fit_file = '%s/%s_SvtBaselineFit.root'%(options.outDir, histo_file[:-5])

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
fitBL.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/svt/SvtBlFits.json'
fitBL.parameters["outDir"] = options.outDir
fitBL.parameters["hybrid"] = options.hybrid
fitBL.parameters["nhitsFitCut"] = options.nhitsFitCut
fitBL.parameters["rebin"] = options.rebin
fitBL.parameters["nPoints"] = options.nPoints

# Sequence which the processors will run.
p.sequence = [fitBL]

p.input_files=[histo_file]
p.output_files = [fit_file]

p.printProcess()
