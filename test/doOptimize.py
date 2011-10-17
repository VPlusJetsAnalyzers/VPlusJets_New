#! /usr/bin/env python

import subprocess
import re

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
(opts, args) = parser.parse_args()

if opts.P > 3:
    print 'precision cannot be better than 10^-3'
    opts.P = 3
cmdRoot = ['root', '-l', '-b', '-q', 'RooWjjFitterNew.C+(0,0.,true,false,false,{0})'.format(opts.Nj)]
cmdGrepChi = ['grep', '^ \*\*\* ']

optVars = { 'fSU' : 0.0, 'fMU' : 0.0 }
## optVars = ['fSU']

from ROOT import TGraph, TF1, gPad, TFile, Double

outf = TFile('optimization{0}.root'.format(opts.Nj), 'recreate')
keepIterating = True
iteration = -1

for optVar in optVars:
    initFile = open(opts.startingFile).readlines()
    for line in initFile:
        found = re.search('^{0} = [ ]*([-0-9\.]*)'.format(optVar), line)
        if found:
            optVars[optVar] = float(found.group(1))

#for iteration in range(0, 2):
while keepIterating:
    iteration += 1
    keepIterating = False
    Npts = 6
    start = 0.
    step = 0.1
    if iteration > 0:
        step = step/10.
    if iteration > 1:
        step = step/2**(iteration-1)
    if step < 10**(-opts.P+1):
        step = 10**(-opts.P+1)
    for optVar in optVars:
        initFile = open(opts.startingFile).readlines()
        for line in initFile:
            found = re.search('^{0} = [ ]*([-0-9\.]*)'.format(optVar), line)
            if found:
                start = float(found.group(1))
        oldVal = start
        if iteration > 0:
            start -= 2*step
        else:
            start = 0.
        if start < 0.:
            start = 0.

        optGraph = TGraph(Npts)
        optGraph.SetName('graph_{0}_{1}'.format(optVar, iteration))
        SetPoints = 0
        minchi2 = 10000.
        minVal = 0.
        for point in range(0, Npts):
            newVal = start + step*point
            theSum = 0.
            for tmpVar in optVars:
                if tmpVar == optVar:
                    theSum += newVal
                else:
                    theSum += optVars[tmpVar]
            if theSum > 1.0:
                continue

            SetPoints += 1
            testFile = [re.sub('^{0} = [ ]*[-0-9\.]*'.format(optVar),
                               '{0} = {1:.3f}'.format(optVar, newVal), x) \
                        for x in initFile]
            print ''.join(testFile)
            tmpInit = open('tmpInit.txt', 'w')
            tmpInit.writelines(testFile)
            tmpInit.close()

            p1 = subprocess.Popen(cmdRoot, stdout=subprocess.PIPE)
            p2 = subprocess.Popen(cmdGrepChi, stdin=p1.stdout,
                                  stdout=subprocess.PIPE)
            p1.stdout.close()
            output = p2.communicate()[0]

            print output
            chi2match = re.search('dof = ([-+]?(\d+(\.\d*)?|\.\d+)([eE][-+]?\d+)?)/(\d+)', output)
            chi2 = float(chi2match.group(1))
            dof = float(chi2match.group(5))
            prob = float(re.search('probability = ([-+]?(\d+(\.\d*)?|\.\d+)([eE][-+]?\d+)?)', output).group(1))
            print 'chi2:', chi2, 'dof:', dof, 'prob:', prob
            if chi2 < minchi2:
                minchi2 = chi2
                minVal = newVal
            optGraph.SetPoint(point, newVal, chi2)

        optGraph.Set(SetPoints)
        bestVal = minVal
        if SetPoints > 2:
            parabolaFit = TF1("parabFit", "x*x++x++1", start, newVal)
            optGraph.Fit(parabolaFit)
            if (parabolaFit.GetParameter(0) > 0):
                bestVal = -1.*parabolaFit.GetParameter(1)/2./parabolaFit.GetParameter(0)
            else:
                bestVal = minVal
        if bestVal > 1.0:
            bestVal = 1.0
        if bestVal < 0.:
            bestVal = 0.
        theSum = 0.
        for tmpVar in optVars:
            if tmpVar == optVar:
                theSum += bestVal
            else:
                theSum += optVars[tmpVar]
        if theSum > 1.0:
            bestVal = minVal

        print 'old minimum:',oldVal,'new minimum:', bestVal
        optVars[optVar] = bestVal
        newStart = [re.sub('^{0} = [ ]*[-0-9\.]*'.format(optVar),
                           '{0} = {1:.3f}'.format(optVar, bestVal), x) \
                    for x in open(opts.startingFile).readlines()]
        newFile = open(opts.startingFile, 'w')
        newFile.writelines(newStart)
        newFile.close()
        optGraph.Write()
        if abs(oldVal-bestVal) > 10**(-opts.P):
            keepIterating = True
        
    ##     optGraph.Draw('ap*')
    ##     gPad.WaitPrimitive()

outf.Close()
