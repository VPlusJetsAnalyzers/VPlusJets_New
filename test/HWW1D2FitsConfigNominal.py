from RooWjj2DFitterPars import Wjj2DFitterPars
from ROOT import kRed, kAzure, kGreen, kBlue, kCyan, kViolet, kGray
import HWWSignalShapes

def theConfig(mu2Pars, el2Pars, **kwargs):
    # (Nj, mH, isElectron = False, initFile = [], includeSignal = True,
    #           MVACutOverride = None):
    print 'mjj',kwargs
    pars = Wjj2DFitterPars()

    pars.MCDirectory = '/eos/uscms/store/user/lnujj/RDtrees_with_8TeV_MVA_v2/'
    # pars.MCDirectory = '/eos/uscms/store/user/lnujj/RDtrees_with_8TeV_MVA/Higgs_22Oct/'
    if ('xrootd' in kwargs) and kwargs['xrootd']:
        pars.MCDirectory = 'root://cmseos:1094/' + pars.MCDirectory
    pars.QCDDirectory = "/uscms_data/d3/ilyao/QCD8TeV/Moriond13/"

    pars.DataDirectory = pars.MCDirectory
    pars.isElectron = kwargs['isElectron']
    if ('initFile' in kwargs):
        pars.initialParametersFile = list(kwargs['initFile'])
    else:
        pars.initialParametersFile = []

    pars.backgrounds = ['diboson', 'WpJ', 'top']
    if ('includeSignal' in kwargs):
        pars.includeSignal = kwargs['includeSignal']
    else:
        pars.includeSignal = False
    pars.signals = ['ggH', 'qqH']
    pars.yieldConstraints = { 'diboson' : 0.10, 'top' : 0.07 }
    #pars.yieldConstraints = {}
    pars.constrainShapes = ['WpJ']
    # if kwargs['mH'] > 200:
    #     pars.constrainShapes = ['WpJ']

    pars.Njets = kwargs['Nj']
    pars.mHiggs = kwargs['mH']

    modePars = mu2Pars
    if pars.isElectron:
        flavorString = 'el'
        if pars.Njets == 3:
            modePars = el3Pars
        else:
            modePars = el2Pars
    else:
        flavorString = 'mu'
        if pars.Njets == 3:
            modePars = mu3Pars
        else:
            modePars = mu2Pars

    pars.cuts = '(ggdevt==%i)&&(fit_status==0)&&(W_mt>30)' % pars.Njets
    pars.cuts = '(fit_status==0)&&(W_mt>30)'
    if ('MVACutOverride' in kwargs) and (kwargs['MVACutOverride'] != None):
        pars.cuts += '&&(%s>%.3f)' % \
            (modePars[pars.mHiggs][0].replace('mu', flavorString), 
             kwargs['MVACutOverride'])
    else:
        pars.cuts += '&&(%s>%.3f)' % \
            (modePars[pars.mHiggs][0].replace('mu', flavorString), 
             modePars[pars.mHiggs][1])
    
    #btag veto
    pars.btagVeto = False
    for i in range(0, 6):
        pars.cuts += '&&((abs(JetPFCor_Eta[%i])>2.4)||' % i + \
            '(JetPFCor_Pt[%i]<30.)||' % i + \
            '(JetPFCor_bDiscriminatorCSV[%i]<0.244))' % i

    # veto boosted topology
    # if pars.mHiggs >= 600:
    #     pars.cuts += '&&(W_pt<200.)'

    # veto vbf
    # pars.cuts += '&&(vbf_event==0)'

    # you need a files entry and a models entry for each of the fit 
    # compoents in backgrounds and signals
    # the files should a list with entries like (filename, Ngen, xsec)
    pars.dibosonFiles = [
        (pars.MCDirectory + 'RD_%s_WW_CMSSW532.root' % (flavorString),
         9450414, 57.1097),
        (pars.MCDirectory + 'RD_%s_WZ_CMSSW532.root' % (flavorString),
         10000267, 32.3161),
        ]
    pars.dibosonModels = [ modePars[pars.mHiggs][5]['diboson'][0] ]

    wpj_kfactor = 1.16
    pars.WpJFiles = [
        # (pars.MCDirectory + 'RD_%s_WpJ_CMSSW532.root' % (flavorString),
        #  18353019+50768992, 36257.2),
        (pars.MCDirectory + 'RD_%s_ZpJ_CMSSW532.root' % (flavorString),
         30209426, 3503.71),
        # (pars.MCDirectory + 'RD_%s_W1Jets_CMSSW532.root' % (flavorString),
        #  19871598, 5400.0*wpj_kfactor),
        (pars.MCDirectory + 'RD_%s_W2Jets_CMSSW532.root' % (flavorString),
         33004921, 1750.0*wpj_kfactor),
        (pars.MCDirectory + 'RD_%s_W3Jets_CMSSW532.root' % (flavorString),
         15059503, 519.0*wpj_kfactor),
        # (pars.MCDirectory + 'RD_%s_W4Jets_CMSSW532.root' % (flavorString),
        #  12842803, 214.0*wpj_kfactor),
        (pars.MCDirectory + 'RD_%s_W4Jets_CMSSW532_old.root' % (flavorString),
         4369420, 214.0*wpj_kfactor),
        ]
    pars.WpJModels = [ modePars[pars.mHiggs][5]['WpJ'][0] ]
    if (len(modePars[pars.mHiggs][5]['WpJ']) > 2):
        pars.WpJAuxModels = [ modePars[pars.mHiggs][5]['WpJ'][2] ]

    pars.topFiles = [
        (pars.MCDirectory + 'RD_%s_TTbar_CMSSW532.root' % (flavorString),
         6893735, 225.197),
        (pars.MCDirectory + 'RD_%s_STopS_Tbar_CMSSW532.root' % (flavorString),
         139974, 1.75776),
        (pars.MCDirectory + 'RD_%s_STopS_T_CMSSW532.root' % (flavorString),
         259960, 3.89394),
        (pars.MCDirectory + 'RD_%s_STopT_Tbar_CMSSW532.root' % (flavorString),
         1935066, 30.0042),
        (pars.MCDirectory + 'RD_%s_STopT_T_CMSSW532.root' % (flavorString),
         3758221, 55.531),
        (pars.MCDirectory + 'RD_%s_STopTW_Tbar_CMSSW532.root' % (flavorString),
         493458, 11.1773),
        (pars.MCDirectory + 'RD_%s_STopTW_T_CMSSW532.root' % (flavorString),
         497657, 11.1773),
        ]
    pars.topModels = [ modePars[pars.mHiggs][5]['top'][0] ]

    pars.QCDFracOfData = 0.05
    pars.QCDFiles = [
        (pars.QCDDirectory + 'RDQCD_WenuJets_Isog0p3NoElMVA_19p2invfb.root',
         1,1), #The events come from the data sideband
        ]
    pars.QCDModels = [27]

    ngen = HWWSignalShapes.NgenHiggs(pars.mHiggs, 'ggH')
    pars.ggHFiles = [
        (pars.MCDirectory + HWWSignalShapes.makeSignalFilename(pars.mHiggs, 
                                                               "ggH",
                                                               pars.isElectron),
         ngen[0], ngen[1]*ngen[2])
        ]
    pars.ggHModels = [ modePars[pars.mHiggs][5]['ggH'][0] ]
    pars.ggHdoSystMult = False
    if pars.mHiggs >= 400:
        pars.ggHInterference = True
        pars.ggHSystMult = 'interf_ggH'


    ngen = HWWSignalShapes.NgenHiggs(pars.mHiggs, 'qqH')
    pars.qqHFiles = [
        (pars.MCDirectory + HWWSignalShapes.makeSignalFilename(pars.mHiggs, 
                                                               "qqH",
                                                               pars.isElectron),
         ngen[0], ngen[1]*ngen[2])
        ]
    pars.qqHModels = [ modePars[pars.mHiggs][5]['qqH'][0] ]

    pars.dibosonPlotting = {'color' : kAzure+8, 'title' : 'WW+WZ'}
    pars.WpJPlotting = { 'color' : kRed, 'title' : 'V+jets'}
    pars.topPlotting = {'color' : kGreen+2, 'title' : 'top'}
    pars.ggHPlotting = {'color' : kBlue, 
                        'title' : "H(%i) #rightarrow WW" % pars.mHiggs}
    pars.qqHPlotting = {'color' : kBlue, 
                        'title' : "H(%i) #rightarrow WW" % pars.mHiggs,
                        'visible' : True}
    pars.QCDPlotting = {'color' : kGray, 'title':'multijet'}

    pars.var = ['Mass2j_PFCor', 'fit_mlvjj']
    pars.varRanges = {'Mass2j_PFCor': (13, 50., 154., []),
                      'fit_mlvjj': (modePars[pars.mHiggs][4]*10, 
                                    modePars[pars.mHiggs][2], 
                                    modePars[pars.mHiggs][3], [])
                      }
    pars.plotRanges = {'Mass2j_PFCor': (13, 50., 154., []),
                      'fit_mlvjj': (modePars[pars.mHiggs][4], 
                                    modePars[pars.mHiggs][2], 
                                    modePars[pars.mHiggs][3], [])
                      }
    pars.varTitles = {'Mass2j_PFCor': 'm_{jj}',
                      'fit_mlvjj' : 'm_{l#nujj}'
                      }
    pars.exclude = {'Mass2j_PFCor' : (66., 66.+32.)}
    pars.doExclude = True
    pars.blind = False

    pars.binData = False
    # pars.binData = True

    return customizeElectrons(pars) if pars.isElectron else \
        customizeMuons(pars)

def customizeElectrons(pars):
    pars.DataFile = pars.DataDirectory + 'RD_WenuJets_DataAllSingleElectronTrigger_GoldenJSON_19p2invfb.root'
    pars.integratedLumi = 19200

    pars.doEffCorrections = True
    pars.effToDo = ['lepton']
    pars.leptonEffFiles = {
        'id': ["EffTable2012/scaleFactor-Run2012ABC-GsfElectronToId.txt"],
        'reco': ["EffTable2012/scaleFactor-Run2012ABC-SCToElectron.txt"],
        'HLT': ["EffTable2012/efficiency-Run2012ABC-WP80ToHLTEle.txt"]
        }
    pars.lumiPerEpoch = [pars.integratedLumi]

    pars.cuts += '&&(abs(JetPFCor_dphiMET[0])>0.8)&&(W_electron_pt>35)'
    return pars

def customizeMuons(pars):
    pars.DataFile = pars.DataDirectory + 'RD_WmunuJets_DataAll_GoldenJSON_19p3invfb.root'
    pars.integratedLumi = 19300

    pars.doEffCorrections = True
    pars.effToDo = ['lepton']
    pars.leptonEffFiles = {
        'id': ["EffTable2012/scaleFactor-Run2012ABC-RecoToIso.txt"],
        'HLT': ["EffTable2012/efficiency-Run2012ABC-IsoToIsoMuHLT.txt"]
        }
    pars.lumiPerEpoch = [pars.integratedLumi]

    pars.cuts += '&&(abs(JetPFCor_dphiMET[0])>0.4)&&(abs(W_muon_eta)<2.1)'
    
    return pars
