import argparse

parser = argparse.ArgumentParser(description="The baseConfig options for hpstr. ")

parser.add_argument('--debug', '-D', action="count", dest="debug", help="Increase debug level.", default=0)

#parser.add_option("-i", "--inFile", type="string", dest="inFilename",
#                  help="Input filename.", metavar="inFilename", default="")
parser.add_argument('--outDir', '-d', type=str, dest="outDir", action='store',
                  help="Specify the output directory.", metavar="outDir", default=".")
parser.add_argument("-o", "--outFile", type=str, dest="outFilename", action='store',
                  help="Output filename.", metavar="outFilename", default="out.root")
parser.add_argument("-t", "--isData", type=int, dest="isData",
                  help="Type of lcio ntuple: 1=data, 0=MC", metavar="isData", default=-1)
parser.add_argument("-y", "--year", type=int, dest="year",
                  help="Select year of the data", metavar="year", default=2019)
parser.add_argument("-n", "--nevents", type=int, dest="nevents",
                  help="Number of events to process", metavar="nevents",default=-1)

parser.add_argument('--infile','-i', type=str, dest="inFilename", metavar='infiles', nargs="+",
                    help="Input files, specify on or more.")


beamE = {}
beamE["2016"] = 2.3
beamE["2019"] = 4.55

bfield = {}
bfield["2016"] = 0.52
bfield["2019"] = 1.034
