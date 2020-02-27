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
