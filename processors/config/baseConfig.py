from optparse import OptionParser

parser = OptionParser()

parser.add_option("-i", "--inFile", type="string", dest="inFilename",
                  help="Input filename.", metavar="inFilename", default="")
parser.add_option("-d", "--outDir", type="string", dest="outDir",
                  help="Specify the output directory.", metavar="outDir", default=".")
parser.add_option("-o", "--outFile", type="string", dest="outFilename",
                  help="Output filename.", metavar="outFilename", default="out.root")
parser.add_option("-t", "--isData", type="int", dest="isData",
                  help="Type of lcio ntuple: 1=data, 0=MC", metavar="isData", default=-1)
parser.add_option("-y", "--year", type="int", dest="year",
                  help="Select year of the data", metavar="year", default=2019)
parser.add_option("-n", "--nevents", type="int", dest="nevents",
                  help="Number of events to process", metavar="nevents",default=-1)


beamE = {}
beamE["2016"] = 2.3
beamE["2019"] = 4.55

bfield = {}
bfield["2016"] = 0.52
bfield["2019"] = 1.034
