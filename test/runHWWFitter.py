#! /usr/bin/env python

#######################################################
## ---> Run using the command line like: 
##       python runHWWFitter.py -m HWWMuonsConfig -j 2 -H 300 -W <N>
######################################################

from optparse import OptionParser

wpjhelp = '''Parameterization for W+jets fit ---
-1 : histograms with floating fMU and fSU
 0 : erf * (power law with two params)
 1 : erf * (power law ith one param) * exponential
 2 : (fermi turn on) * (power law with two params)
 3 : (fermi turn on) * exponential
 4 : exponential * (power law with one param)
 5 : exponential * (power law with two params)
 6 : erf * (power law with one param)
everything else : same as zero
'''

parser = OptionParser()
parser.add_option('-b', action='store_true', dest='noX', default=False,
                  help='no X11 windows')
parser.add_option('-j', '--Njets', dest='Nj', default=2, type='int',
                  help='Number of jets.')
parser.add_option('-i', '--init', dest='startingFile',
                  default='',
                  help='File to use as the initial template')
parser.add_option('-d', '--dir', dest='mcdir', default='',
                  help='directory to pick up the W+jets shapes')
parser.add_option('-m', '--mode', default="HWWconfig", dest='modeConfig',
                  help='which config to select look at HWWconfig.py for an '+ \
                  'example.  Use the file name minus the .py extension.')
parser.add_option('-H', '--mH', dest='mH', default=400, type='int',
                  help='Higgs Mass Point')
parser.add_option('-s', '--syst', dest='syst', type='int', default=0,
                   help='alpha systematic 0: none, 1: down, 2: up')
parser.add_option('-W', '--WpJ', dest='ParamWpJ', type='int',
                  default=-1, help=wpjhelp)
parser.add_option('--debug', action='store_true', dest='debug', default=False,
                  help='stop when things aren\'t good')
parser.add_option('--compile', action='store_true', dest='compile', 
                  default=False, help='stop when done compiling')
parser.add_option('--mvaCut', dest='mvaCut', type='float',
                  help='override the mva cut with this value')
parser.add_option('--alpha', dest='alpha', type='float',
                  help='override the alpha with this value')
parser.add_option('--qgCut', dest='qgCut', type='float',
                  help='cut value on the product of QGLikelihood')
parser.add_option('--lumi', dest='lumi', type='float',
                  help='integrated lumi in pb^-1')
(opts, args) = parser.parse_args()

import pyroot_logon

assert (opts.syst >= 0)
assert (opts.syst <= 2)

#import HWWwideSideband

from ROOT import gPad, TFile, Double, Long, gROOT, TCanvas
config = __import__(opts.modeConfig)

#gROOT.ProcessLine('.L RooWjjFitterParams.h+')
gROOT.ProcessLine('.L RooWjjFitterUtils.cc+')
gROOT.ProcessLine('.L RooWjjMjjFitter.cc+')
from ROOT import RooWjjMjjFitter, RooFitResult, RooWjjFitterUtils, \
     RooMsgService, RooFit, TLatex, TMatrixDSym, RooArgList, RooArgSet, gPad, \
     kBlue, TH1D, TMath, gDirectory, RooWjjFitterParams, kRed, kGray
from math import sqrt
import HWWSignalShapes
import pulls

if not opts.debug:
    RooMsgService.instance().setGlobalKillBelow(RooFit.WARNING)
fitterPars = config.theConfig(opts.Nj, opts.mcdir, opts.startingFile, opts.mH,
                              opts.mvaCut, opts.qgCut)

fitterPars.WpJfunction = opts.ParamWpJ
#fitterPars.truncRange = True
if opts.ParamWpJ<0:
    fitterPars.smoothingOrder = 0

#fitterPars.smoothingOrder = 0

if opts.mH < 250:
    fitterPars.binEdges.erase(fitterPars.binEdges.begin())
    fitterPars.minMass = fitterPars.binEdges[0]
    fitterPars.minFit = fitterPars.minMass
if (opts.mH == 400):
    fitterPars.binEdges.insert(fitterPars.binEdges.begin(), 40.)
    fitterPars.minMass = fitterPars.binEdges[0]
    fitterPars.minFit = fitterPars.minMass

if opts.lumi:
    fitterPars.intLumi = opts.lumi
    
if fitterPars.includeMuons and fitterPars.includeElectrons:
    modeString = ''
elif fitterPars.includeMuons:
    modeString = 'Muon'
elif fitterPars.includeElectrons:
    modeString = 'Electron'
else:
    modeString = ''

theFitter = RooWjjMjjFitter(fitterPars)

if opts.compile:
    print 'compile complete'
    assert(False)

#theFitter.getWorkSpace().Print()
theFitter.makeFitter((opts.ParamWpJ>=0))

totalPdf = theFitter.getWorkSpace().pdf('totalPdf')
#theFitter.getWorkSpace().Print()
theFitter.getWorkSpace().var('nDiboson').setConstant(False)

# if opts.ParamWpJ < 0:
#     theFitter.getWorkSpace().var('fMU').setConstant(True)
#     theFitter.getWorkSpace().var('fSU').setConstant(True)

# theFitter.getWorkSpace().var('nDiboson').setConstant()
#theFitter.getWorkSpace().var('nTTbar').setConstant()
# theFitter.getWorkSpace().var('nSingleTop').setConstant()
# theFitter.getWorkSpace().var('nZjets').setConstant()
#theFitter.getWorkSpace().var('nWjets').setConstant()
theFitter.getWorkSpace().var('nQCD').setConstant()

if (opts.Nj == 3):
    theFitter.getWorkSpace().var('nTTbar').setConstant()
if (opts.mH <= 300) and (opts.Nj == 2):
    theFitter.getWorkSpace().var('nTTbar').setConstant()

theFitter.loadData()
theFitter.resetYields()
if fitterPars.includeMuons:
    theFitter.getWorkSpace().var('nQCD').setConstant()

fr = theFitter.fit()

tries = 1
ndf = Long(fr.floatParsFinal().getSize()-5)
while (fr.covQual() < 2) and (tries < 3):
    print "Fit didn't converge well.  Will try again."
    # theFitter.getWorkSpace().var('nDiboson').setConstant()
    theFitter.getWorkSpace().var('nTTbar').setConstant()
    # theFitter.getWorkSpace().var('nSingleTop').setConstant()
    theFitter.getWorkSpace().var('nQCD').setConstant()
    # theFitter.getWorkSpace().var('nZjets').setConstant()
    #theFitter.getWorkSpace().var('nWjets').setConstant()
    fr = theFitter.fit()
    ndf = Long(fr.floatParsFinal().getSize())
    tries += 1

mass = theFitter.getWorkSpace().var(fitterPars.var)
iset = RooArgSet(mass)
nexp = theFitter.makeFitter().expectedEvents(iset)

ndf2 = Long(ndf)
print 'Corrected chi2'
#chi2 = ndf*1.0
chi2 = theFitter.computeChi2(ndf)
print 'Raw chi2'
#chi2Raw = ndf2*1.0
chi2Raw = theFitter.computeChi2(ndf2, False)

mf = theFitter.stackedPlot()
mf.SetName("Mjj_Stacked")
## sf = theFitter.residualPlot(mf, "h_background", "dibosonPdf", False)
# pf = theFitter.residualPlot(mf, "h_total", "", True)
pf = pulls.createPull(mf.getHist('theData'), mf.getCurve('h_total'))
pf.SetName("Mjj_Pull")
## lf = theFitter.stackedPlot(True)

l = TLatex()
l.SetNDC()
l.SetTextSize(0.035)
l.SetTextFont(42)

cs = TCanvas("cs", "stacked")
mf.Draw()
# l.DrawLatex(0.66, 0.55,
#             '#chi^{{2}}/dof = {0:0.3f}/{1}'.format(chi2, ndf)
#             )
pyroot_logon.cmsPrelim(cs, fitterPars.intLumi/1000)
cs.Print('H{2}_Mjj_{0}_{1}jets_Stacked.pdf'.format(modeString, opts.Nj, opts.mH))
cs.Print('H{2}_Mjj_{0}_{1}jets_Stacked.png'.format(modeString, opts.Nj, opts.mH))

# if (fr.covQual() != 3):
#     print "Fit did not converge with a good error matrix. Bailing out."
#     assert(False)

c4 = TCanvas("c4", "pull")
pf.Draw('ap')
c4.SetGridy()
c4.Update()
pf.SetMinimum(-5.)
pf.SetMaximum(5.)
pf.GetXaxis().SetLimits(fitterPars.minMass, fitterPars.maxMass)
pf.GetXaxis().SetTitle('m_{jj} (GeV)')
pf.GetYaxis().SetTitle('pull (#sigma)')
pyroot_logon.cmsPrelim(c4, fitterPars.intLumi/1000)
c4.Print('H{2}_Mjj_{0}_{1}jets_Pull.pdf'.format(modeString, opts.Nj, opts.mH))
c4.Print('H{2}_Mjj_{0}_{1}jets_Pull.png'.format(modeString, opts.Nj, opts.mH))

if (TMath.Prob(chi2Raw,  ndf2) < 0.05):
    print '**DANGER** The fit probability is low **DANGER**'
    if opts.debug:
        assert(False)

#assert(False)

mass.setRange('signal', fitterPars.minTrunc, fitterPars.maxTrunc)
#yields = theFitter.makeFitter().coefList()
finalPars = fr.floatParsFinal()
yields = RooArgList(finalPars)
yields.add(fr.constPars())
sigInt = theFitter.makeFitter().createIntegral(iset,iset,'signal')
sigFullInt = theFitter.makeFitter().createIntegral(iset,iset)
print "allBkg","sigInt",sigInt.getVal(),"fullInt",sigFullInt.getVal(),\
      "ratio",sigInt.getVal()/sigFullInt.getVal()
dibosonInt = theFitter.makeDibosonPdf().createIntegral(iset,iset,'signal')
dibosonFullInt = theFitter.makeDibosonPdf().createIntegral(iset,iset)
WpJPdf = theFitter.makeWpJPdf()
WpJInt = WpJPdf.createIntegral(iset, iset, 'signal')
WpJFullInt = WpJPdf.createIntegral(iset, iset)
#WpJPdf.Print("v")
print "WpJ","sigInt",WpJInt.getVal(),"fullInt",WpJFullInt.getVal(),\
      "ratio",WpJInt.getVal()/WpJFullInt.getVal()
ttbarInt = theFitter.makettbarPdf().createIntegral(iset, iset, 'signal')
ttbarFullInt = theFitter.makettbarPdf().createIntegral(iset, iset)
SingleTopInt = theFitter.makeSingleTopPdf().createIntegral(iset, iset, 'signal')
SingleTopFullInt = theFitter.makeSingleTopPdf().createIntegral(iset, iset)
QCDInt = theFitter.makeQCDPdf().createIntegral(iset, iset, 'signal')
QCDFullInt = theFitter.makeQCDPdf().createIntegral(iset, iset)
ZpJInt = theFitter.makeZpJPdf().createIntegral(iset, iset, 'signal')
ZpJFullInt = theFitter.makeZpJPdf().createIntegral(iset, iset)
print "ZpJ","sigInt",ZpJInt.getVal(),"fullInt",ZpJFullInt.getVal(),\
      "ratio",ZpJInt.getVal()/ZpJFullInt.getVal()
## print "*** yield vars ***"
## yields.Print("v")
covMatrix = TMatrixDSym(fr.covarianceMatrix())
corMatrix = TMatrixDSym(fr.correlationMatrix())

sig2 = 0.
## print '\nCorrelation matrix\n%-10s' % (' '),
## for v1 in range(0, finalPars.getSize()):
##     if finalPars[v1].GetName()[0] == 'n':
##         print '%10s' % (finalPars[v1].GetName()),
## print
covVars = []
for v1 in range(0, covMatrix.GetNrows()):
##     if finalPars[v1].GetName()[0] == 'n':
##         print '%-10s' % (finalPars[v1].GetName()),
    covVars.append(finalPars[v1].GetName())
    for v2 in range(0, covMatrix.GetNcols()):
        if ((finalPars[v1].GetName())[0] == 'n') and \
               ((finalPars[v2].GetName())[0] == 'n'):
##             print '% 10.2g' % (corMatrix(v1, v2)),
            sig2 += covMatrix(v1, v2)
##     print

print 'sig2:', sig2
usig2 = 0.
totalYield = 0.
sigYield = 0.
sigErrs = {}

sigYieldFilename = 'last_H%i_%s_%iJets_signalYield.txt' % (opts.mH, 
                                                           modeString, opts.Nj)
sigYieldsFile = open(sigYieldFilename, 'w')

WpJNonPoissonError = 0

print
print '-------------------------------'
print 'Yields in signal box'
print '-------------------------------'
for i in range(0, yields.getSize()):
    theName = yields.at(i).GetName()
    if theName[0] == 'n':
        totalYield += yields.at(i).getVal()
        theIntegral = 1.
        if (theName == 'nDiboson'):
            theIntegral = dibosonInt.getVal()/dibosonFullInt.getVal()
        elif (theName == 'nWjets'):
            theIntegral = WpJInt.getVal()/WpJFullInt.getVal()
            if (yields.at(i).getError()**2 > yields.at(i).getVal()):
                WpJNonPoissonError = sqrt(yields.at(i).getError()**2 - \
                                              yields.at(i).getVal())
            else:
                WpJNonPoissonError = 0.
        elif (theName == 'nTTbar'):
            theIntegral = ttbarInt.getVal()/ttbarFullInt.getVal()
        elif (theName == 'nSingleTop'):
            theIntegral = SingleTopInt.getVal()/SingleTopFullInt.getVal()
        elif (theName == 'nQCD'):
            theIntegral = QCDInt.getVal()/QCDFullInt.getVal()
        elif (theName == 'nZjets'):
            theIntegral = ZpJInt.getVal()/ZpJFullInt.getVal()

        y = yields.at(i).getVal()*theIntegral
        if (yields.at(i).getVal() > 0) and \
               (yields.at(i).getError()**2 > yields.at(i).getVal()):
            yerr = sqrt(yields.at(i).getError()**2 - yields.at(i).getVal())
            yerr *= theIntegral
            yerr = sqrt(yerr**2 + y)
            #print '*',
        else:
            yerr = yields.at(i).getError()*theIntegral
        yieldString = '%s = %0.0f +/- %0.0f' % (theName, y, yerr)
        print yieldString
        sigErrs[theName] = yerr
        sigYield += y
    else:
        yieldString = '%s = %0.3f +/- %0.3f' % (theName,
                                                yields.at(i).getVal(),
                                                yields.at(i).getError())
    sigYieldsFile.write(yieldString + '\n')

sigYieldsFile.close()

if (sig2 - totalYield) > 0:
    sigSig2 = (sqrt(sig2-totalYield)/totalYield*sigYield)**2
else:
    sigSig2 = 0.
sigSig2 += sigYield
print '-------------------------------'
print 'total yield = %0.0f +/- %0.0f' % (sigYield, sqrt(sigSig2))
print '-------------------------------'

print 'total Signal yield by fraction: %.0f +/- %.0f' % (totalYield*sigInt.getVal()/sigFullInt.getVal(), sqrt(sigSig2))
sigSig2 -= sigYield
## print 'sig box all:',totalYield*sigInt.getVal()/sigFullInt.getVal(),\
##       'err:',sqrt(sig2)*sigInt.getVal()/sigFullInt.getVal()
print 'fractional error on background yield not from Poisson stats = %0.3f' % (sqrt(sigSig2)/sigYield)
print 'fractional error on background yield not from using W+jets only = %0.3f' % (WpJNonPoissonError/totalYield)

print '\nCorrelation matrix\n%-10s' % (' '),
for v1 in sigErrs:
    if v1 in covVars:
        print '%10s' % v1,
print
for v1 in sigErrs:
    if not v1 in covVars:
        continue
    print '%-10s' % (v1),
    for v2 in sigErrs:
        if not v2 in covVars:
            continue
        print '% 10.2g' % (fr.correlation(v1,v2)),
    print

fr.Print()
nll=fr.minNll()
print '***** nll = ',nll,' ***** \n'
print 'total yield: {0:0.0f} +/- {1:0.0f}'.format(totalYield, sqrt(sig2))

# assert(False)

cWpJ = TCanvas('cWpJ', 'W+jets shape')
pars4 = config.the4BodyConfig(fitterPars, opts.mH, opts.syst, opts.alpha)
pars4.mHiggs = opts.mH
pars4.wHiggs = HWWSignalShapes.HiggsWidth[opts.mH]
pars4.initParamsFile = sigYieldFilename
fitter4 = RooWjjMjjFitter(pars4)

fitter4.makeFitter((opts.ParamWpJ>=0))
fitter4.loadData()
fitter4.make4BodyPdf(theFitter)
fitter4.loadParameters(sigYieldFilename)

## assert(False)

mf4 = fitter4.stackedPlot(False, RooWjjMjjFitter.mlnujj)
mf4.SetName("Mlvjj_Stacked")
## sf4 = fitter4.residualPlot(mf4, "h_background", "dibosonPdf", False)
# pf4 = fitter4.residualPlot(mf4, "h_total", "", True)
pf4 = pulls.createPull(mf4.getHist('theData'), mf4.getCurve('h_total'))
pf4.SetName("Mlvjj_Pull")
lf4 = fitter4.stackedPlot(True, RooWjjMjjFitter.mlnujj)
lf4.SetName("Mlvjj_log")

fitUtils = RooWjjFitterUtils(pars4)
iwt = 0
if (opts.mH >= 500):
    iwt = 1
sigHists = HWWSignalShapes.GenHiggsHists(pars4, opts.mH, fitUtils, iwt = iwt)

extraFactor = 2.
otherdata = HWWSignalShapes.NgenHiggs(opts.mH, 'HWW')
SigVisual = TH1D(sigHists['HWW'])
SigVisual.Print()
SigVisual.SetName('SigVisual')
SigVisual.SetLineColor(kBlue)
SigVisual.SetLineWidth(3)
SigVisual.Scale(pars4.intLumi*extraFactor*otherdata[1]*otherdata[2]/2.)
SigVisual.Print()
SigVisualLog = TH1D(SigVisual)
SigVisualLog.SetName("SigVisualLog")

c4body = TCanvas('c4body', '4 body stacked')
mf4.addTH1(SigVisual, "hist")

tmpLeg = mf4.findObject('theLegend')
tmpLeg.AddEntry(SigVisual, "H(%d)#times%.0f" % (opts.mH, extraFactor), "l")

if (pars4.model >= 1):
    c = fitter4.getWorkSpace().var('c')
    c.Print()
    cNom = c.getVal()

    c.setVal(cNom + c.getError())
    mf4_up = fitter4.stackedPlot(False, RooWjjMjjFitter.mlnujj)
    c.setVal(cNom - c.getError())
    mf4_down = fitter4.stackedPlot(False, RooWjjMjjFitter.mlnujj)

    h_total_up = mf4_up.getCurve('h_total')
    h_total_up.SetName('h_total_up')
    h_total_up.SetLineColor(kGray+3)
    h_total_up.SetLineStyle(2)
    h_total_down = mf4_down.getCurve('h_total')
    h_total_down.SetName('h_total_down')
    h_total_down.SetLineColor(kGray+3)
    h_total_down.SetLineStyle(2)

    mf4.addPlotable(h_total_up, "l")
    mf4.addPlotable(h_total_down, "l")
    
    tmpLeg.AddEntry(h_total_up, "bkg. syst.", "l")

mf4.Draw()
pyroot_logon.cmsPrelim(c4body, pars4.intLumi/1000)

c4body.Update()
c4body.Print('H{2}_Mlvjj_{0}_{1}jets_Stacked.pdf'.format(modeString, opts.Nj, opts.mH))
c4body.Print('H{2}_Mlvjj_{0}_{1}jets_Stacked.png'.format(modeString, opts.Nj, opts.mH))

## assert(False)

c4body2 = TCanvas("c4body2", "4 body stacked_log")
c4body2.SetLogy()
#lf4.addTH1(SigVisualLog, "hist")
tmpLeg = lf4.findObject('theLegend')
tmpLeg.AddEntry(SigVisualLog, "H(%d)#times%.0f" % (opts.mH, extraFactor),
                "l")
lf4.Draw()
SigVisualLog.Draw("histsame")
pyroot_logon.cmsPrelim(c4body2, pars4.intLumi/1000)
c4body2.Print('H{2}_Mlvjj_{0}_{1}jets_Stacked_log.pdf'.format(modeString,
                                                              opts.Nj,
                                                              opts.mH))
c4body2.Print('H{2}_Mlvjj_{0}_{1}jets_Stacked_log.png'.format(modeString,
                                                              opts.Nj,
                                                              opts.mH))

c4body4 = TCanvas("c4body4", "4 body pull")
pf4.Draw('ap')
c4body4.SetGridy()
c4body4.Update()
pf4.SetMinimum(-5.)
pf4.SetMaximum(5.)
pf4.GetXaxis().SetLimits(pars4.minMass, pars4.maxMass)
pf4.GetXaxis().SetTitle("m_{l#nujj} (GeV)")
pf4.GetYaxis().SetTitle('pull (#sigma)')
pyroot_logon.cmsPrelim(c4body4, pars4.intLumi/1000)
c4body4.Update()
c4body4.Print('H{2}_Mlvjj_{0}_{1}jets_Pull.pdf'.format(modeString, opts.Nj,
                                                       opts.mH))
c4body4.Print('H{2}_Mlvjj_{0}_{1}jets_Pull.png'.format(modeString, opts.Nj,
                                                       opts.mH))

sbf = cWpJ.FindObject("SideBandPlot")
print sbf
if not sbf:
    sbf = gDirectory.Get("SideBandPlot")
    print sbf
    sbf.Print()

c.setVal(cNom + c.getError())
fitter4.getWorkSpace().pdf("WpJ4BodyPdf").plotOn(sbf, RooFit.LineStyle(2),
                                                 RooFit.Name('syst_up'),
                                                 RooFit.LineColor(kBlue+1))

c.setVal(cNom - c.getError())
fitter4.getWorkSpace().pdf("WpJ4BodyPdf").plotOn(sbf, RooFit.LineStyle(2),
                                                 RooFit.Name('syst_down'),
                                                 RooFit.LineColor(kBlue+1))

cWpJ.cd()
sbf.Draw()
pyroot_logon.cmsLabel(cWpJ, pars4.intLumi/1000, prelim=True)
cWpJ.Update()
cWpJ.Modified()
cWpJ.Update()

cWpJ.Print('H%i_Mlvjj_%s_%ijets_WpJShape.pdf' % (opts.mH, modeString,
                                                 opts.Nj))
cWpJ.Print('H%i_Mlvjj_%s_%ijets_WpJShape.png' % (opts.mH, modeString,
                                                 opts.Nj))
# cWpJ.Print('H%i_Mlvjj_%s_%ijets_WpJShape.root' % (opts.mH, modeString,
#                                                   opts.Nj))

sbf.SetMinimum(1e-4)
cWpJ.SetLogy()
cWpJ.Update()
cWpJ.Modified()
cWpJ.Update()
cWpJ.Print('H%i_Mlvjj_%s_%ijets_WpJShape_log.pdf' % (opts.mH, modeString,
                                                     opts.Nj))
cWpJ.Print('H%i_Mlvjj_%s_%ijets_WpJShape_log.png' % (opts.mH, modeString,
                                                     opts.Nj))
# cWpJ.Print('H%i_Mlvjj_%s_%ijets_WpJShape_log.root' % (opts.mH, modeString,
#                                                       opts.Nj))

fitter4.getWorkSpace().Print()

print 'shape file created'
ShapeFile = TFile('H{2}_{1}_{0}Jets_Fit_Shapes.root'.format(opts.Nj,
                                                           modeString,
                                                           opts.mH),
                  'recreate')

h_total = mf4.getCurve('h_total')

theData = mf4.getHist('theData')

fitter4.getWorkSpace().Write("ws4")
theFitter.getWorkSpace().Write('ws2')

h_total.Write()
theData.Write()

h_total_up.Write()
h_total_down.Write()

for mode in sigHists:
    sigHists[mode].Write()

sigHists['HWW'].Print()
nomIntegral = sigHists['HWW'].Integral()
if iwt == 1:
    pars4up = RooWjjFitterParams(pars4)
    if (opts.Nj == 2):
        pars4up.cuts = pars4.cuts.replace('interferencenominal', 
                                          'interferenceup')
    fitUtilsUp = RooWjjFitterUtils(pars4up)
    sigHistsUp = HWWSignalShapes.GenHiggsHists(pars4up, opts.mH, fitUtilsUp, 
                                               iwt = 2)
    sigHistsUp['HWW'].SetName(sigHistsUp['HWW'].GetName() + '_up')
    ShapeFile.cd()
    sigHistsUp['HWW'].Write()
    sigHistsUp['HWW'].Print()
    upIntegral = sigHistsUp['HWW'].Integral()
    pars4down = RooWjjFitterParams(pars4)
    if (opts.Nj == 2):
        pars4down.cuts = pars4.cuts.replace('interferencenominal', 
                                            'interferencedown')
    fitUtilsDown = RooWjjFitterUtils(pars4down)
    sigHistsDown = HWWSignalShapes.GenHiggsHists(pars4down, opts.mH, 
                                                 fitUtilsDown, iwt = 3)
    sigHistsDown['HWW'].SetName(sigHistsDown['HWW'].GetName() + '_down')
    ShapeFile.cd()
    sigHistsDown['HWW'].Write()
    sigHistsDown['HWW'].Print()
    downIntegral = sigHistsDown['HWW'].Integral()

# HiggsHist.Write()
# VBFHiggsHist.Write()
# TauNuHiggsHist.Write()

ShapeFile.cd()
mf.Write()
pf.Write()
mf4.Write()
pf4.Write()
lf4.Write()
sbf.Write()

SigVisual.Write()
SigVisualLog.Write()

ShapeFile.Close()

if iwt == 1:
    print "down norm change:", (downIntegral/nomIntegral)
    print "up norm change:", (upIntegral/nomIntegral)
