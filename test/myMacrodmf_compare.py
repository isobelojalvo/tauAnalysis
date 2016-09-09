import ROOT as rt
import CMS_lumi, tdrstyle
import array
from sys import argv,exit

rt.gROOT.SetBatch(True) #Running in batch mode prevents keeps plots from being drawn; much quicker
decayModes = ["h^{#pm}","h^{#pm}#pi^{0}","h^{#pm}h^{#mp}","h^{#pm}h^{#mp}#pi^{0}","h^{#pm}h^{#mp}h^{#pm}", "11"]
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
tdrstyle.logy = True
tdrstyle.logx = False
tdrstyle.setTDRStyle()
colors=[1,2,4,6,8,12]  #Add more colors for >5 sets of points in a single plot
styles=[20,21,22,23,33,34]  #Add more point styles if needed

#change the CMS_lumi variables (see CMS_lumi.py)
CMS_lumi.writeExtraText = 1
CMS_lumi.extraText = "Simulation Preliminary"
#CMS_lumi.lumiText = "Simulation: Z* #rightarrow #tau#tau"
CMS_lumi.lumiText = "ggH #rightarrow #tau#tau"
tdrstyle.logy = 1
iPos = 11

H_ref = 700; 
W_ref = 800; 
W = W_ref
H  = H_ref
bin_edge = 50
binning_array = [20,50]
while bin_edge < 1000:
    bin_edge += 50
    binning_array.append(bin_edge)
while bin_edge < 1600:
    bin_edge += 75
    binning_array.append(bin_edge)
while bin_edge < 2000:
    bin_edge += 100
    binning_array.append(bin_edge)
print binning_array

def make_plot(tree, variable, selection, binning, title=''):
    ''' Plot a variable using draw and return the histogram '''
    draw_string = "%s>>htemp(%s)" % (variable, ", ".join(str(x) for x in binning))
    tree.Draw(draw_string, selection, "goff")
    output_histo = rt.gDirectory.Get("htemp").Clone()
    inclusiveBinning = array.array('d', binning_array)
    output_histo = output_histo.Rebin(len(binning_array)-1,"rebinned",inclusiveBinning)
    return output_histo

def make_efficiency(denom, num):
    ''' Make an efficiency graph with the style '''
    eff = rt.TGraphAsymmErrors(num, denom)
    eff.SetMarkerStyle(20)
    eff.SetMarkerSize(1.75)
    eff.SetLineColor(rt.kBlack)
    return eff

def make_num(ntuple, variable,binning,dmf):
    num = make_plot(
        ntuple, variable,
        "(recoDecayMode==%d)&&(decayMode==30||decayMode==31||decayMode==32)"%dmf,
        binning
    )
    return num

def make_denom(ntuple, variable,binning,dmf):
    denom = make_plot(
        ntuple, variable,
        "decayMode==30||decayMode==31||decayMode==32",
        binning
    )
    return denom

def produce_efficiency(ntuple, variable,binning,color,style,dmf):
    denom = make_denom(ntuple, variable,binning,dmf)
    num = make_num(ntuple,variable,binning,dmf)
    l1 = make_efficiency(denom,num)
    l1.SetMarkerColor(color)
    l1.SetMarkerStyle(style)
    return l1

# references for T, B, L, R
T = 0.08*H_ref
B = 0.15*H_ref 
L = 0.15*W_ref
R = 0.04*W_ref

def CompareEfficiencies(ntuple,dmfs,variable,binning,filename,xtitle,ymin,ymax):
	canvas = rt.TCanvas("c2","c2",50,50,W,H)
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

	hists = []
	for i in range(len(dmfs)):
		hists.append(produce_efficiency(ntuple, variable,binning,colors[i],styles[i],dmfs[i]))

	titles = "h; " + xtitle +"; Reconstruction ratio"
	h =  rt.TH1F("h",titles,binning[0],binning[1],binning[2])

	h.SetMaximum(ymax)
	h.SetMinimum(ymin)

	xAxis = h.GetXaxis()
	xAxis.SetNdivisions(6,5,0)
	xAxis.SetTitleOffset(1.1)

	yAxis = h.GetYaxis()
	yAxis.SetNdivisions(10)
	yAxis.SetTitleOffset(1.1)

	h.Draw()

	for i in range(len(hists)):
		if i==0:
			hists[i].Draw('pe')
		else:
			hists[i].Draw('pesame')

	#draw the lumi text on the canvas
	CMS_lumi.CMS_lumi(canvas, 0, iPos)

	canvas.cd()
	canvas.Update()
	canvas.RedrawAxis()
	frame = canvas.GetFrame()
	frame.Draw()

	legend = rt.TLegend(0.20,0.77 ,0.95,0.90, "Reconstructed #tau_{h} decay mode", "brNDC")
	legend.SetNColumns(3)	
    	legend.SetFillColor(rt.kWhite)	#No background color to legend
	legend.SetLineColor(0)	#No line around legend
    	legend.SetBorderSize(1)
	for i in range(len(dmfs)):
		legend.AddEntry(hists[i],decayModes[i],'pe')
    	legend.Draw()


	#update the canvas to draw the legend
	canvas.Update()
	saveas = saveWhere + filename + '.png'
	canvas.SaveAs(saveas)

#Call function to create/save plots
CompareEfficiencies(byLooseCmbIso3,[0,1,5,6,10],'tauPt', [100,0,2000], 'tau_eff_dmfcompare_looseIso_pT',"3-prong #tau^{gen}_{h} p_{T} (GeV)",1e-4,10)
CompareEfficiencies(byMedCmbIso3,[0,1,5,6,10],'tauPt', [100,0,2000], 'tau_eff_dmfcompare_medIso_pT',"3-prong #tau^{gen}_{h} p_{T} (GeV)",1e-4,10)
CompareEfficiencies(byTightCmbIso3,[0,1,5,6,10],'tauPt', [100,0,2000], 'tau_eff_dmfcompare_tightIso_pT',"3-prong #tau^{gen}_{h} p_{T} (GeV)",1e-4,10)
CompareEfficiencies(byNone,[0,1,5,6,10],'tauPt', [100,0,2000], 'tau_eff_dmfcompare_none_pT',"3-prong #tau^{gen}_{h} p_{T} (GeV)",1e-4,10)
