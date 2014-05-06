from optparse import OptionParser

parser = OptionParser()
parser.add_option('-b', action='store_true', dest='noX', default=False,
                  help='no X11 windows')
(opts, args) = parser.parse_args()

import pyroot_logon
import re
from ROOT import TFile, gPad, TCanvas

cs = TCanvas('cs', 'stacked')
cp = TCanvas('cp', 'pull')
cp.SetGridy()

for fname in args:
    print fname
    fname_minusPath = fname.split('/')[-1]
    fname_path = '/'.join(fname.split('/')[0:-1])
    fname_parts = fname_minusPath.split('_')
    if len(fname_path) > 0:
        fname_path += '/'
    print 'filename:', fname_minusPath, 'path:', fname_path

    f = TFile(fname)
    match = re.search(r'\d+', fname_parts[0])
    mH = int(match.group(0))
    mjj_plot = f.Get('Mass2j_PFCor_stacked')
    cs.cd()
    cs.SetLogy(False)
    mjj_plot.Draw()
    gPad.Update()
    gPad.Print('%s_%s_mjj_stacked.pdf' % (fname_parts[0], fname_parts[1]))
    gPad.Print('%s_%s_mjj_stacked.png' % (fname_parts[0], fname_parts[1]))

    mjj_pull = f.Get('Mass2j_PFCor_pull')
    cp.cd()
    mjj_pull.Draw('ap')
    gPad.Update()
    gPad.Print('%s_%s_mjj_pull.pdf' % (fname_parts[0], fname_parts[1]))
    gPad.Print('%s_%s_mjj_pull.png' % (fname_parts[0], fname_parts[1]))

    mWW_plot = f.Get('fit_mlvjj_plot_stacked')
    cs.cd()
    mWW_plot.Draw()
    gPad.Update()
    gPad.Print('%s_%s_mWW_stacked.pdf' % (fname_parts[0], fname_parts[1]))
    gPad.Print('%s_%s_mWW_stacked.png' % (fname_parts[0], fname_parts[1]))
    if mH > 400:
        cs.SetLogy(True)
        mWW_plot.SetAxisRange(0.1, 1e5, 'Y')
        gPad.Update()
        gPad.Print('%s_%s_mWW_stacked_log.pdf' % \
                       (fname_parts[0], fname_parts[1]))
        gPad.Print('%s_%s_mWW_stacked_log.png' % \
                       (fname_parts[0], fname_parts[1]))


    mWW_pull = f.Get('mWW_pull')
    cp.cd()
    mWW_pull.Draw('ap')
    gPad.Update()
    gPad.Print('%s%s_%s_mWW_pull.pdf' % (fname_path, fname_parts[0], 
                                         fname_parts[1]))
    gPad.Print('%s%s_%s_mWW_pull.png' % (fname_path, fname_parts[0], 
                                         fname_parts[1]))
