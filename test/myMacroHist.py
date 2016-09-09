import ROOT as rt
import CMS_lumi, tdrstyle
import array
from sys import argv,exit
import math
import numpy as np

rt.gROOT.SetBatch(True) #Running in batch mode prevents keeps plots from being drawn; much quicker

######## File #########
if len(argv) < 2:
   print 'Usage:python plot.py RootFile.root label[optional]'
   exit()

infile = argv[1]
ntuple_file = rt.TFile(infile,"READ")

byLooseCmbIso3 = ntuple_file.Get("byLooseCombinedIsolationDeltaBetaCorr3Hits/Ntuple")
byMedCmbIso3 = ntuple_file.Get("byMediumCombinedIsolationDeltaBetaCorr3Hits/Ntuple")
byTightCmbIso3 = ntuple_file.Get("byTightCombinedIsolationDeltaBetaCorr3Hits/Ntuple")
byNone = ntuple_file.Get("byNone/Ntuple")

######## LABEL & SAVE WHERE #########
if len(argv)>2:
   saveWhere='~/private/output/2prong/'+argv[2]+'_'
else:
   saveWhere='~/private/output/2prong/'

######## Style Choices ########
tdrstyle.ygrid = True
tdrstyle.logy = False
tdrstyle.logx = False
tdrstyle.setTDRStyle()
colors=[1,2,4,6,8,12]  #Add more colors for >5 sets of points in a single plot
styles=[20,21,22,23,33,34]  #Add more point styles if needed

#change the CMS_lumi variables (see CMS_lumi.py)
CMS_lumi.writeExtraText = 1
CMS_lumi.extraText = "Simulation Preliminary"
#CMS_lumi.lumiText = "Simulation: Z* #rightarrow #tau#tau"
CMS_lumi.lumiText = "ggH #rightarrow #tau#tau"
iPos = 11

H_ref = 700 
W_ref = 800
W = W_ref
H  = H_ref
xscale = 1.
yscale = 1
# references for T, B, L, R
T = 0.08*H_ref
B = 0.14*H_ref 
L = 0.1455555*W_ref
R = 0.16*W_ref

def CompareTracks(ntuple,filename,title,variable,drawMode, maxDigit, limits, axisTitle):
    canvas = rt.TCanvas("c2","c2",50,50,W,H)
    canvas.DrawFrame(1e-6,1e-6,1,1)
    canvas.SetFillColor(0)
    canvas.SetBorderMode(0)
    canvas.SetFrameFillStyle(0)
    canvas.SetFrameBorderMode(0)
    canvas.SetLeftMargin( L/W )
    canvas.SetRightMargin( R/W )
    canvas.SetTopMargin( T/H )
    canvas.SetBottomMargin( B/H )
    canvas.SetTickx(0)
    canvas.SetTicky(1)
    #xbins,ybins = RebinLogHist2(limits)
    if tdrstyle.logx == True:
        xbins,ybins = RebinLogHist2(limits)
        h = rt.TH2F("h","hist",len(xbins)-1,xbins,len(ybins)-1,ybins)
    elif variable[:-1] == 'dxy' or variable[:-1]== 'dz':
        h = rt.TH2F("h","hist", 50,limits[0]*xscale,limits[1]*xscale,50,limits[2]*yscale,limits[3]*yscale)
    else:
        #h = rt.TH2F("h","hist",len(xbins)-1,xbins,len(ybins)-1,ybins)
        h = rt.TH1F("h","hist", 30,limits[0],limits[1])
        
    #ntuple.Draw(variable[:-1] + "3:" + variable + ">>h", "recoTrack==1", drawMode)
    ntuple.Draw("tauEta>>h", "recoTrack==0", drawMode)
    #ntuple.Draw("trackDpT:trackDR>>h", "", drawMode)
    #ntuple.Draw("trackDpt>>h", "recoTrack==1", drawMode)
    h.SetStats(rt.kFALSE)
    h.SetMarkerStyle(1)
    xAxis = h.GetXaxis()
    xAxis.SetNdivisions(10)
    xAxis.SetTitleOffset(1.1)
    if variable[-1:] == '1':
        xAxis.SetTitle("1^{st} track " + axisTitle )
        if variable[:-1]=='dR':
            xAxis.SetTitle("dR(1^{st} track, #tau_{h}^{reco}) (cm)")
    else:
        xAxis.SetTitle("2^{nd} track " + axisTitle)
        if variable[:-1]=='dR':
            xAxis.SetTitle("dR(2^{nd} track, #tau_{h}^{reco}) (cm)")
    xAxis.SetTitle("#tau_{h}^{gen} #eta")
        
        

    yAxis = h.GetYaxis()
    yAxis.SetNdivisions(10)
    yAxis.SetTitleOffset(1.12)
    #yAxis.SetTitle("1^{st} track # Hits")
    if variable[:-1]=='dR':
        yAxis.SetTitle("dR(3^{rd} track, #tau_{h}^{reco}) (cm)")
    #yAxis.SetTitle("3^{rd} (failed) track " + axisTitle)
    #yAxis.SetTitle("1^{st} track " + axisTitle)
    #xAxis.SetTitle("dR(#pi^{#pm}_{gen}, #pi^{#pm}_{reco}) (cm)")
    #yAxis.SetTitle("|p_{T}^{#pi^{#pm}_{gen}} - p_{T}^{#pi^{#pm}_{reco}})| (GeV)")
    
    h.Draw(drawMode)
    #if maxDigit != 0:
    #    rt.TGaxis.SetMaxDigits(maxDigit)
    text = rt.TText(0.77-len(title)*0.01,0.955,title + " isolation")
    text.SetTextAlign(22)
    text.SetTextColor(rt.kBlack)
    text.SetTextFont(43)
    text.SetTextSize(30)
    text.SetNDC()
    text.Draw()
    #draw the lumi text on the canvas
    CMS_lumi.CMS_lumi(canvas, 0, iPos)

    canvas.cd()
    canvas.Update()
    canvas.RedrawAxis()
    frame = canvas.GetFrame()
    frame.Draw()

    #save image of plot
    saveas = saveWhere + filename + drawMode + 'tauEta0.png'
    canvas.SaveAs(saveas)

def CompareCuts(ntuple,filename,title,variable,maxDigit,limits, axisTitle):
    #CompareTracks(ntuple,filename+'1',title,variable+'1','colz',maxDigit,limits, axisTitle)
    CompareTracks(ntuple,filename+'1',title,variable+'1','',maxDigit,limits, axisTitle)
    #CompareTracks(ntuple,filename+'2',title,variable+'2','colz',maxDigit,limits, axisTitle)
    #CompareTracks(ntuple,filename+'2',title,variable+'2','',maxDigit,limits, axisTitle)

def RebinLogHist2(limits):
    xbins = []
    ybins = []
    for i in np.arange(limits[0],limits[1],10):
        xbins.append(i)
    #for i in np.arange(math.log10(limits[0]),math.log10(limits[1])+0.1, 0.1):
    #    xbins.append(math.pow(10.0,i))
    for i in np.arange(math.log10(limits[2]),math.log10(limits[3])+0.1, 0.1):
        ybins.append(math.pow(10.0,i))
    xbins = array.array('d',xbins)
    ybins = array.array('d',ybins)
    return xbins, ybins

def MakePlots(variable, limits, axisTitle):
    CompareCuts(byLooseCmbIso3, variable+'loose', 'Loose', variable, 2, limits, axisTitle)
    CompareCuts(byMedCmbIso3, variable+'med', 'Medium', variable, 2, limits, axisTitle)
    CompareCuts(byTightCmbIso3, variable+'tight', 'Tight', variable,  2, limits, axisTitle)
    CompareCuts(byNone, variable+ 'none', 'No', variable, 2, limits, axisTitle)

#Call function to create/save plots
#MakePlots('dxyErr', [1e-4,1,1e-4,1], "#delta dxy (cm)")
#MakePlots('pT', [1e-1,1e3,1e-1,1e3], "p_{T} (GeV)")
#MakePlots('dzErr', [1e-4,10,1e-4,10], "#delta dz (cm)")
#MakePlots('dR', [1e-5,10,1e-5,10], "dR (cm)")
#MakePlots('dxy', [-0.005,0.005,-0.005,0.005], "dxy (mm)")
#MakePlots('dz', [-0.005,0.005,-0.005,0.005], "dz (mm)")

#MakePlots('dxy', [0,800,-0.05,0.05], "dxy (mm)")
#MakePlots('dz', [0,800,-0.05,0.05], "dz (mm)")

#MakePlots('numHits', [0,30,0,30], '# Hits')
#MakePlots('numPixHits', [0,7,0,7], '# Pixel hits')

#MakePlots('dxyErr', [0,800,1e-4,1], "dxy Err. (cm)")
#MakePlots('dzErr', [0,800,1e-4,10], "dz Err. (cm)")

MakePlots('numHits', [-2.3,2.3,0,30], "dxy Err. (cm)")
#MakePlots('numHits', [0,800,0,30], "dxy Err. (cm)")
#MakePlots('dxy', [1e-5,1,1e-3,1e3], "dxy (mm)")
#MakePlots('dz', [0,800,-0.05,0.05], "dz (mm)")
