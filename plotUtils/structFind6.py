import ROOT as r
from ROOT import TCanvas,TH1F,TH2F,TH2D,TLegend,THStack
import os,sys,glob
import numpy as np
import argparse

def makeHTML(outDir,filename,title,plots,branchnames,mode):#,selection):
    os.chdir(outDir)
    outfilebase = os.path.split(outDir)[1]
    f = open(filename+'.html',"w+")
    f.write("<!DOCTYPE html\n")
    f.write(" PUBLIC \"-//W3C//DTD HTML 3.2//EN\">\n")
    f.write("<html>\n")
    f.write("<head><title>"+ title +" </title></head>\n")
    f.write("<body bgcolor=\"EEEEEE\">\n")
    f.write("<table border=\"0\" cellspacing=\"5\" width=\"100%\">\n")
    for i in range(0,len(plots)):
        pname = ""
        offset = 1
        if i==0 or i%2==0:
            f.write("<tr>\n")
        if mode==0:
            f.write("<td width=\"10%\"><a target=\"_blank\" href=\"" + plots[i] + "\"><img src=\"" + plots[i] + "\" alt=\"" + plots[i] + "\" title=\"" + pname + "\" width=\"85%\" ></a></td>\n")
        if mode==1:
            if plots[i][len(plots[i])-1]=="l":
                f.write("<td width=\"10%\"><a target=\"_blank\" href=\"" + plots[i] + "\">"+branchnames[i]+"</a></td>\n")	
            else:
                f.write("<td width=\"10%\"><a target=\"_blank\" href=\"" + plots[i] + "\"><img src=\"" + plots[i] + "\" alt=\"" + plots[i] + "\" title=\"" + pname + "\" width=\"85%\" ></a></td>\n")
        if i==offset:
            f.write("</tr>\n")
        elif (i>offset and (i-offset)%2==0) or i==len(plots):
            f.write("</tr>\n")
    f.write("</table>\n")
    f.write("</body>\n")
    f.write("</html>")
    f.close()
    path_parent=os.path.dirname(os.getcwd())
    os.chdir(path_parent)

def buildLegend(canvas,x1=0.50, y1=0.60,x2=0.70,y2=0.8,textsize=0.025,separation=0.1,fill=0,border=0):
    #canvas.cd()
    legend = canvas.BuildLegend(x1,y1,x2,y2)
    legend.Draw()
    legend.SetTextSize(textsize)
    #legend.SetEntrySeparation(separation)
    #legend.SetFillStyle(fill)
    legend.SetFillColorAlpha(r.kGray,0.7)
    legend.SetBorderSize(border)

def format1DPlot(plot, name, title = "",linecolor = 1, linewidth = 2, linestyle = 1):
    plot.SetLineWidth(linewidth)
    plot.SetLineColor(linecolor)
    plot.SetLineStyle(linestyle)
    plot.SetName(name)
    if title != "":
        plot.SetTitle(title)

def getStatsYPositions(nplots):
    start = 1.0
    end = 0.1
    height = (start-end)/nplots
    if height > 0.1:
        height = 0.1
    #ypositions = list(range(start,end,height))
    ypositions = np.arange(end,start,height)
    ypositions=np.flipud(ypositions)
    print(ypositions)
    return ypositions,height


def setStatsBox(plot,xpos=0.9,ypos=0.9,height=0.2,width=0.1,linecolor=r.kBlack,color=r.kWhite,alpha=0.8):
    r.gPad.Update()
    stats = plot.GetListOfFunctions().FindObject("stats")
    stats.SetFillColorAlpha(color,alpha)
    stats.SetLineColor(linecolor)
    r.gStyle.SetStatY(ypos)
    r.gStyle.SetStatH(height)
    r.gStyle.SetStatW(width)
    r.gStyle.SetStatX(xpos)
    r.gPad.Update()

def savePlotAsPNG(plot, plot_dir, drawOptions=""):
    c = r.TCanvas(plot.GetName(),plot.GetName(),1800,900)
    c.cd()
    plot.Draw("%s"%(drawOptions))
    setStatsBox(plot,xpos=0.8,ypos=0.8)
    c.Update()
    c.SaveAs("%s/%s.png"%(plot_dir,plot.GetName()))
    #r.gPad.Update()
    c.Close()

def overlay1DPlots(plots,outf,c,canvas_name,legend_names = []):
    #statsbox formatting
    statspositions,height = getStatsYPositions(len(plots))

    ymax = -99999
    for i,plot in enumerate(plots):
        ymaxi = plot.GetMaximum()
        if ymaxi > ymax:
            ymax = ymaxi


    #c = r.TCanvas("%s"%(canvas_name),"%s"%(canvas_name),1800,1000)
    c.cd()

    #set yaxis
    for i,plot in enumerate(plots):
        plot.GetYaxis().SetRangeUser(0.00001,ymax*1.1)
        if i < 1:
            plot.Draw("hist")
            setStatsBox(plot,ypos=statspositions[i],height=height,linecolor=colors[i])
        else:
            plot.Draw("histsames")
            setStatsBox(plot,ypos=statspositions[i],height=height,linecolor=colors[i])

    if len(legend_names) > 0:
        for i, plot in enumerate(plots):
            plot.SetTitle(legend_names[i])

    buildLegend(c)
    plots[0].SetTitle(canvas_name)
    c.SaveAs(outf+"/"+canvas_name+".png","png")
    #r.gPad.Update()
    c.Close()

parser = argparse.ArgumentParser(description="The baseConfig options for plotHistDir")

parser.add_argument("-i","--inFileBase",type=str,dest="inf",action='store',help="input file base, use when files are in the same directory", default=None)
parser.add_argument("-o","--outFileBase",type=str,dest="outf",action='store',help="Generates Pictures and HTML Webpage Here", default=None)
parser.add_argument("-r","--ratioEnable",type=int,dest="ratio",action='store',help="Enables Ratio Curves", default=0)
parser.add_argument("-mc","--mcFileBase",type=str,dest="mc",action='store',help="Enables MC and Takes in Curves", default="Nope")
parser.add_argument("-l","--list",nargs=4,type=float,dest="leglist",action='store',help="Relocates the Legend",default=[.73,.32,.97,.53])

options = parser.parse_args()
inf=options.inf
outf=options.outf
R=(options.ratio==1)#or(not(options.mc=='Nope'))
MC=options.mc
leglist=options.leglist
print(leglist)
#treeNum=options.treeNum
#branchNum=options.branchNum

colors = [r.kBlack,r.kRed,r.kBlue,r.kGreen+2,r.kOrange-2]
#legends=["",""]

#print(os.listdir(inf))

inFileList=os.listdir(inf)
if (not(options.mc=='Nope')):
	mcFileList=os.listdir(MC)
if R:
	inFileList=inFileList[:2]
r.gROOT.SetBatch(1)

DFS=r.TFile(inf+"/"+inFileList[0])#DirectoryForSize
SubDirs=[DFS.Get(DFS.GetListOfKeys().At(i).GetName()) for i in range(0,len(DFS.GetListOfKeys()))]
SubDirNames=[DFS.GetListOfKeys().At(i).GetName() for i in range(0,len(DFS.GetListOfKeys()))]
BranchNames=[[SubDirs[i].GetListOfKeys().At(j).GetName() for j in range(len(SubDirs[i].GetListOfKeys()))] for i in range(len(SubDirs))]
#OneD=[]
for i in range(len(DFS.GetListOfKeys())):
	OneD=[]
	for  j in range(len(SubDirs[i].GetListOfKeys())):
		#print(BranchNames[i][j])
		c=r.TCanvas()
		leg = TLegend(leglist[0],leglist[1],leglist[2],leglist[3])#TLegend(.73,.32,.97,.53)
		DFSnow=r.TFile(inf+"/"+inFileList[0])
		Entry=DFSnow.Get(SubDirNames[i]+"/"+BranchNames[i][j])	
		is2d=(type(Entry)==TH2D)
		#print(is2d)
		try:
			if (not(is2d))and(not(options.mc=='Nope')):
				ths1=THStack(BranchNames[i][j],BranchNames[i][j])
				for I in range(len(mcFileList)):
					DFSnow=r.TFile(MC+"/"+mcFileList[I])
					Entry=DFSnow.Get(SubDirNames[i]+"/"+BranchNames[i][j])
					Entry.SetDirectory(0)
					ths1.Add(Entry)
				ths1.Draw("stack")
				print("hello")
		except:
			print("Failed at "+BranchNames[i][j])
		if R and not(is2d):
			c.Divide(1,2)
			c.cd(1)		
		
		plots=[]
		for I in range(len(inFileList)):
			DFSnow=r.TFile(inf+"/"+inFileList[I])
			Entry=DFSnow.Get(SubDirNames[i]+"/"+BranchNames[i][j])	
			if is2d:
				Entry.SetTitle(inFileList[I])
				Entry.Draw("colz")
				c.Print(outf+"/pic"+str(i)+"_"+str(j)+"_"+str(I)+".png","png")
				c.Clear()
				continue
			Entry.SetDirectory(0)
			Entry.SetLineColor(colors[I%5])
			if options.mc=='Nope':
				leg.AddEntry(Entry,inFileList[I],"L")
			plots.append(Entry)
			#Entry.Draw("colz same")
			#overlay1DPlots()	

		try:
			if R and not(is2d):
				c.cd(2)
				f1=r.TFile(inf+"/"+inFileList[0])
				Entry1=f1.Get(SubDirNames[i]+"/"+BranchNames[i][j])
				Entry1.SetDirectory(0)
				f2=r.TFile(inf+"/"+inFileList[1])
				Entry2=f2.Get(SubDirNames[i]+"/"+BranchNames[i][j])
				Entry2.Divide(Entry1)
				#print("hello")
				Entry2.Draw("colz")
		except:
			print("Failed on 1D Histogram for "+BranchNames[i][j])
		if is2d:	
			makeHTML(outf,BranchNames[i][j],BranchNames[i][j],["pic"+str(i)+"_"+str(j)+"_"+str(I)+".png" for I in range(len(inFileList))],[],0)
			print("hello")
			OneD.append(0)
		else:
			overlay1DPlots(plots,outf,c,BranchNames[i][j],inFileList)
			#leg.Draw()
			#c.Print(outf+"/"+BranchNames[i][j]+".png","png")
			c.Clear()
			OneD.append(1)
	filetype=[".html",".png"]
	#ThreeD=OneD
	#print(OneD)
	#print(len(BranchNames[0]))
	print([BranchNames[i][j]+"_"+str(OneD[j])+"_"+filetype[OneD[j]] for j in range(len(BranchNames[i]))])
	makeHTML(outf,SubDirNames[i],SubDirNames[i],[BranchNames[i][j]+filetype[OneD[j]] for j in range(len(BranchNames[i]))],BranchNames[i],1)
makeHTML(outf,"master","Master",[SubDirNames[i]+".html" for i in range(len(SubDirNames))],SubDirNames,1)
