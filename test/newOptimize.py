#! /usr/bin/env python

import re
import sys

from optparse import OptionParser

parser = OptionParser()
parser.add_option('-b', action='store_true', dest='noX', default=False,
                  help='no X11 windows')
parser.add_option('-j', '--Njets', dest='Nj', default=2, type='int',
                  help='Number of jets.')
parser.add_option('-i', '--init', dest='startingFile',
                  default='TestWjjFitParams.txt',
                  help='File to use as the initial template')
parser.add_option('-p', '--precision', dest='P', default=3, type='int',
                  help='precision to find minimum 10^-P')
parser.add_option('-d', '--dir', dest='mcdir', default='',
                  help='directory to pick up the W+jets shapes')
(opts, args) = parser.parse_args()

if opts.P > 3:
    print 'precision cannot be better than 10^-3'
    opts.P = 3

optVars = { 'fSU' : 0.0, 'fMU' : 0.0 }
## optVars = ['fSU']

import pyroot_logon
from WjjFitterConfigs import HWWconfig
from ROOT import TGraph, TF1, gPad, TFile, Double, Long, gROOT
## gROOT.ProcessLine('.L RooWjjFitterParams.h+');
gROOT.ProcessLine('.L RooWjjFitterUtils.cc+');
gROOT.ProcessLine('.L RooWjjMjjFitter.cc+');
from ROOT import RooWjjMjjFitter, RooFitResult, \
     RooMsgService, RooFit

RooMsgService.instance().setGlobalKillBelow(RooFit.WARNING)
tmpInitFile = 'tmpInit.txt'

fitterPars = HWWconfig(opts.Nj, opts.mcdir, tmpInitFile)
theFitter = RooWjjMjjFitter(fitterPars)


Npts = 6

def optimizeVar (optVar, start, step, iteration):

    optGraph = TGraph(Npts)
    optGraph.SetName('graph_{0}_{1}'.format(optVar, iteration))
    SetPoints = 0
    minchi2 = 10000.
    minVal = 0.
    for point in range(0, Npts):
        newVal = start + step*point
##         theSum = sumPts(optVar, newVal)
##         if theSum > 1.0:
##             continue

        writeValToFile(optVar, newVal, opts.startingFile, tmpInitFile)

        printPts(optVar, newVal)

        fr = theFitter.fit()
        nll = fr.minNll()
        chi2 = Double(0.)
        ndf = Long(fr.floatParsFinal().getSize())
        theFitter.computeChi2(chi2, ndf)
        print 'chi2:', chi2, 'dof:', ndf, 'nll:',nll
        if nll < minchi2:
            minchi2 = nll
            minVal = newVal
        optGraph.SetPoint(SetPoints, newVal, nll)
        SetPoints += 1
        sys.stdout.flush()

    optGraph.Set(SetPoints)
    bestVal = minVal
    if SetPoints > 2:
        parabolaFit = TF1("parabFit", "x*x++x++1", start, newVal)
        optGraph.Fit(parabolaFit)
        if (parabolaFit.GetParameter(0) > 0):
            bestVal = -1.*parabolaFit.GetParameter(1)/2./parabolaFit.GetParameter(0)
        else:
            bestVal = minVal
    print 'extrapolated optimum for',optVar,':',bestVal
    if bestVal > 1.0:
        bestVal = 1.0
    if bestVal < -1.:
        bestVal = -1.
##     theSum = sumPts(optVar, bestVal)
##     if theSum > 1.0:
##         bestVal = minVal
    outf.cd()
    optGraph.Write()

    return round(bestVal,3)

def readValFromFile(optVar, filename):
    compliment = optVar.replace('U','D')
    initFile = open(filename).readlines()
    retVal = 0.
    compVal = 0.
    for line in initFile:
        found = re.search('^{0} = [ ]*([-0-9\.]*)'.format(optVar), line)
        if found:
            retVal = float(found.group(1))
##             print 'found',optVar,':',retVal
        found = re.search('^{0} = [ ]*([-0-9\.]*)'.format(compliment),line)
        if found:
            compVal = float(found.group(1))
##             print 'found',compliment,':',compVal
    if (compVal > retVal):
        retVal = -1*compVal
##     print 'looking for',optVar,'found:',retVal
    return retVal
    
def writeValToFile(optVar, newVal, filename, outname = None):
    if outname == None:
        outname = filename
    compliment = optVar.replace('U', 'D')
    if (newVal >= 0):
        newStart = [re.sub('^{0} = [ ]*[-0-9\.]*'.format(optVar),
                           '{0} = {1:.3f}'.format(optVar, newVal), x) \
                    for x in open(filename).readlines()]
        newStart = [re.sub('^{0} = [ ]*[-0-9\.]*'.format(compliment),
                           '{0} = {1:.3f}'.format(compliment, 0.), x) \
                    for x in newStart]
    else:
        newStart = [re.sub('^{0} = [ ]*[-0-9\.]*'.format(compliment),
                           '{0} = {1:.3f}'.format(compliment, -1*newVal), x) \
                    for x in open(filename).readlines()]
        newStart = [re.sub('^{0} = [ ]*[-0-9\.]*'.format(optVar),
                           '{0} = {1:.3f}'.format(optVar, 0.), x) \
                    for x in newStart]
    newFile = open(outname, 'w')
    newFile.writelines(newStart)
    newFile.close()

def sumPts(optVar, newVal):
    theSum = 0.
    for tmpVar in optVars:
        if tmpVar == optVar:
            theSum += newVal
        else:
            theSum += optVars[tmpVar]
    return theSum

def printPts(optVar, newVal):
    for tmpVar in optVars:
        print tmpVar,'=',
        if tmpVar == optVar:
            print newVal,
        else:
            print optVars[tmpVar],
        print ',',
    print
    
outf = TFile('optimization{0}.root'.format(opts.Nj), 'recreate')
keepIterating = True
iteration = 0

start = 0.
step = 0.1
for optVar in optVars:
    optVars[optVar] = readValFromFile(optVar, opts.startingFile)

print optVars

optVar = 'fSU'
start = optVars[optVar]
oldVal = start

start -= 2*step

if start < -1.:
    start = -1.

bestVal = optimizeVar(optVar, start, step, iteration)

print 'old minimum:',oldVal,'new minimum:', bestVal
optVars[optVar] = bestVal
writeValToFile(optVar, bestVal, opts.startingFile)
sys.stdout.flush()
maxIterations = 9

while (keepIterating) and (iteration < maxIterations):
    iteration += 1
    keepIterating = False
    if iteration > 1:
        step = round(step/2., 3)
##     if iteration > 2:
##         step = step/2**(iteration-1)
    if step < 10**(-opts.P)*3.:
        step = 10**(-opts.P)*3.
    for optVar in optVars:

        start = optVars[optVar]
        oldVal = start
        
        start -= 2*step

        if start < -1.:
            start = -1.

        bestVal = optimizeVar(optVar, start, step, iteration)

        print 'old minimum:',oldVal,'new minimum:', bestVal
        optVars[optVar] = bestVal
        writeValToFile(optVar, bestVal, opts.startingFile)
        if abs(oldVal-bestVal) > 10**(-opts.P):
            keepIterating = True
        sys.stdout.flush()

    ##     optGraph.Draw('ap*')
    ##     gPad.WaitPrimitive()

outf.Close()

print '*** final optimization:',
printPts(' ', 0.)
