#! /bin/env python

import subprocess

def runCommand(cmd, logname = None):
    print ' '.join(cmd)
    if logname:
        logfile = open(logname, 'w')
        errname = subprocess.STDOUT
    else:
        errname = None
        logfile = None
    subprocess.call(cmd, stdout = logfile, stderr = errname)

from optparse import OptionParser

parser = OptionParser()
parser.add_option('--electrons', dest='isElectron', action='store_true',
                  default=False, help='do electrons instead of muons')
parser.add_option('--mva', dest='mvaCut', type='float',
                  help='override cut value for mva')
parser.add_option('--doShapes', dest='doShape', action='store_true',
                  default=False, help='do the shapes fits')
parser.add_option('--doFit', dest='doFit', action='store_true',
                  default=False, help='do the spectrum fit')
parser.add_option('--expLimit', type='int', dest='limit',
                  help='number of toys for expected limit')
parser.add_option('--xrootd', dest='xrootd', action='store_true',
                  help='use xrootd file opening.')
parser.add_option('--obsLimit', dest='obsLimit', action='store_true',
                  default=False,
                  help='calculate observed limit too')

(opts, args) = parser.parse_args()

masspts = [ 170, 180, 190, 200, 250, 300, 350, 400, 450, 500, 550, 600 ]
if len(args) > 0:
    masspts = args

print opts
for mH in masspts:
    print "Higgs mass: %s" % mH
    cmd = ['python', 'run1D2FitsShapeFits.py', '--mH', str(mH)]
    flavStr = 'mu'
    if opts.isElectron:
        cmd.append('--electrons')
        flavStr = 'el'
    if opts.mvaCut:
        cmd.extend(['--mva', str(opts.mvaCut)])
    if opts.xrootd:
        cmd.append('--xrootd')
    if opts.doShape:
        runCommand(cmd, 'HWW%s_%s_shapes.txt' % (mH, flavStr))
        # runCommand(['python', 'fixErfParams.py', '1D2FitsParameters/WpJHWW%sParameters_%smWW.txt' % (mH, 'el_' if opts.isElectron else '')])
    if opts.doFit:
        cmd = ['python', 'runA1D2FitsFit.py', '--mH', str(mH)]
        if opts.isElectron:
            cmd.append('--electrons')
        if opts.mvaCut:
            cmd.extend(['--mva', str(opts.mvaCut)])
        if opts.xrootd:
            cmd.append('--xrootd')
        if opts.limit:
            cmd.extend(['--expLimit', str(opts.limit)])
        if opts.obsLimit:
            cmd.append('--obsLimit')
        runCommand(cmd, 'HWW%s_%s_fit.txt' % (mH, flavStr))