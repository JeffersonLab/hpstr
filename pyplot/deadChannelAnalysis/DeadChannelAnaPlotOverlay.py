import ROOT as r
import numpy as np
import utilities as utils
import os,sys
import SvtCondAnaTools as tools
def OptParsing():

    from optparse import OptionParser
    parser=OptionParser()
    parser.add_option("--id1", dest="inputDir1", help="inputDir1/2",default="")
    parser.add_option("--id2", dest="inputDir2", help="inputDir2/2", default="")
    (options, args) = parser.parse_args()
    return (options)

def Overlay2Histos(histoNames,path_to_File, inputDir1, inputDir2,input_file_ext,legend):
    path_to_File=path_to_File
    input_file_ext=input_file_ext

    inFile1 = path_to_File+options.inputDir1+options.inputDir1[:-1]+input_file_ext
    inFile2 = path_to_File+options.inputDir2+options.inputDir2[:-1]+input_file_ext

    hcanv=[]
    histos1=[]
    histos2=[]
    legend=[]
    print "Getting List of Histo Names"
    f1 = r.TFile.Open(inFile1)
    f2 = r.TFile.Open(inFile2)

    for i in range(len(histoNames)):
        hcanv.append(r.TCanvas(str(i), "c", 1800, 800))
        histos1.append(f1.Get(histoNames[i]+"_h"))
        histos1[i].SetMaximum(40)
        histos1[i].SetMinimum(0)
        histos1[i].SetTitle(histoNames[i]+": "+outFilename+";RMS [ADC Units]; Number of Strips")
        histos2.append(f2.Get(histoNames[i]+"_h"))
        histos2[i].SetMaximum(40)
        histos2[i].SetMinimum(0)

        outFile.cd()
        

        hcanv[i].cd()
        hcanv[i].SetName(histoNames[i]+"_h")
        legend.append(r.TLegend(0.7, 0.8, 0.9,0.73))
        legend[i].AddEntry(histos1[i], inputDir1, "l")
        legend[i].AddEntry(histos2[i], inputDir2, "l")

        histos1[i].SetLineColor(1)
        histos1[i].Draw()

        histos2[i].SetLineColor(2)
        histos2[i].Draw("SAME")
        utils.InsertText("",[], line=0.85, xoffset=0.75, Hps=True)
        legend[i].Draw()
        hcanv[i].Write()
        hcanv[i].SaveAs("RMS_analysis/SavedImages/Histos/h_Canvas_%s.png"%(histoNames[i]))
    f1.Close()
    f2.Close()


def Overlay2Plots(hybridNames, path_to_File, inputDir1, inputDir2,input_file_ext, legend):
    path_to_File=path_to_File
    input_file_ext=input_file_ext
    inFile1 = path_to_File+options.inputDir1+options.inputDir1[:-1]+input_file_ext
    inFile2 = path_to_File+options.inputDir2+options.inputDir2[:-1]+input_file_ext


    gr1=[]
    gr2=[]
    legend=[]
    gcanv=[]
    f1 = r.TFile.Open(inFile1)
    f2 = r.TFile.Open(inFile2)

    for i in range(len(hybridNames)):
        gcanv.append(r.TCanvas(str(i), "c", 1800,800))
        legend.append(r.TLegend(0.7, 0.8, 0.9,0.73))

        gcanv[i].cd()
        gcanv[i].SetName(hybridNames[i]+"_g")

        gr1.append(f1.Get(hybridNames[i]+"_g"))
        gr1[i].SetLineColor(1)
        gr1[i].SetTitle(hybridNames[i]+": "+outFilename+";Strip Number;RMS [ADC Units]")


        gr2.append(f2.Get(hybridNames[i]+"_g"))
        gr2[i].SetLineColor(2)

        legend[i].AddEntry(gr1[i], inputDir1, "l")
        legend[i].AddEntry(gr2[i], inputDir2, "l")

        outFile.cd()

        gr1[i].Draw()
        gr2[i].Draw("SAME")
        utils.InsertText("",[], line=0.85, xoffset=0.75, Hps=True)
        legend[i].Draw()
        gcanv[i].Write()
        gcanv[i].SaveAs("RMS_analysis/SavedImages/Plots/g_Canvas_%s.png"%(hybridNames[i]))

    f1.Close()
    f2.Close()

def Legend():
    #Legend Position
    upperY=0.8
    linesep=0.07
    lowerY=upperY - linesep
    minX = .7
    maxX = minX+0.2
    leg=r.TLegend(minX,upperY,maxX,lowerY)
    leg.SetBorderSize(0)
    leg.SetFillColor(0)
    leg.SetTextSize(0.02)

    return leg
#####################################################################################################################

path_to_File=""
input_file_ext="_RMSanalysis.root"

options=OptParsing()
inFile1 = options.inputDir1[:-1]
inFile2 = options.inputDir2[:-1]
hybridNames=tools.BuildHybridNames()

r.gROOT.SetBatch(r.kTRUE)

outFile_dir="RMS_analysis/"
outFilename=options.inputDir1[:-1]+"_"+options.inputDir2[:-1]
outFile=r.TFile(outFile_dir+outFilename+"_overlay.root", "RECREATE")

legend=Legend()

Overlay2Histos(hybridNames,path_to_File, inFile1, inFile2,input_file_ext,legend)
Overlay2Plots(hybridNames, path_to_File, inFile1, inFile2, input_file_ext,legend)



