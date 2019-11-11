import ROOT as r
import numpy as np
from copy import deepcopy
import utilities as utils
import SvtCondAnaTools as svtc



from optparse import OptionParser
parser=OptionParser()
parser.add_option("--id", dest="inputDir", help="inputDirectory",default="")
#parser.add_option("--inF",dest="inputFile",help="inputFile",default="")
#parser.add_option("--outF",dest="outputFile",help="outputFile",default="rmsPlots.root")
(options, args) = parser.parse_args()

in_path_to_File=""
run = options.inputDir[:-1]
in_file_ext="_anaSvtCond.root"
inFile = r.TFile(in_path_to_File+options.inputDir+run+in_file_ext)

out_path_to_File=""
out_file_ext="_testing.root"
outFilename=out_path_to_File+options.inputDir+run+out_file_ext
outFile = r.TFile(outFilename,"RECREATE")


hybrid_names=svtc.BuildHybridKeys()
sample0Histos=svtc.DeepCopy(inFile, hybrid_names, "_hh")
scADCyps=svtc.GetYPs(sample0Histos)

#PNG_file_path="Images/"

#Dead vs Alive channel analysis
ratioAlive=[]
deadChannels=[]
avgRMS=[]
rmsofRMS=[]
for hName in scADCyps:
     rtemp, dctemp = svtc.RatioDeadAlive(150, scADCyps[hName])
     ratioAlive.append(rtemp)
     deadChannels.append(dctemp)
     avgRMS.append(svtc.AvgRMS(scADCrms[i],cmax))
     rmsofRMS.append(svtc.RMSrms(scADCrms[i],cmax))
layer, module = svtc.swModuleMapper()
svtc.Histo2D(outFileName, "Alive Channels", "Percentage of Alive Channels for "+run, "Layer", "Module", 14, 0.5, 14.5, 4, -0.5,3.5,layer, module, ratioAlive, "RECREATE", False)
svtc.Histo2D(outFileName, "Average RMS", "Averae RMS across Hybrid for "+run, "Layer", "Module", 14, 0.5, 14.5, 4, -0.5,3.5,layer, module, avgRMS, "UPDATE", False)
svtc.Histo2D(outFileName, "RMS of RMS", "RMS of the RMS across Hybrid for "+run, "Layer", "Module", 14, 0.5, 14.5, 4, -0.5,3.5,layer, module, rmsofRMS, "UPDATE", False)




#Create Plots and Histograms for each Hybrid
for i in range(len(scADCrms)):
   # if i==0:
    #    update="RECREATE"
     #   end = False
    if i<len(scADCrms):
        update="UPDATE"
        end=False
    if i==len(scADCrms):
        end = True
    svtc.Histo1D(outFile, hybrid_names[i]+"_h", hybrid_names[i]+"_"+run+"_h", "RMS [ADC Units]", "Number of Strips", 250, 0.5, 1000.5, scADCrms[i], update, end)
    svtc.DrawGraph(outFile, hybrid_names[i]+"_g", hybrid_names[i]+"_"+run+"_g;"+"Strip Number;RMS [ADC Units]", 640, scADCrms[i])
    svtc.Histo1D(outFile, "Dead_Channels_"+hybrid_names[i], "Dead Channels for "+hybrid_names[i]+run,"Channel", "Dead", 640, 0, 640, deadChannels[i],update, end)

outFile.Close()
