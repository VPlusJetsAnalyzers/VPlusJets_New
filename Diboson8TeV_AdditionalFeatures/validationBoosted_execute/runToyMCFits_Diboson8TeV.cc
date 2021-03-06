/*******************************************************************
 * Project: CMS detector at the CERN
 *
 * Package: Presently in the user code
 *
 *
 * Authors:
 *
 *   Osipenkov, Ilya, Texas A&M - ilyao@fnal.gov
 *
 * Description: Use to constuct, fit and analyze toy datasets
 *
 ********************************************************************/

#include <iostream>
#include <fstream>
#include <strstream>
#include <vector>
#include <TFile.h>
#include <TTree.h>
#include <TNtuple.h>
#include <TString.h>
#include <TVector.h>
#include <TMatrix.h>
#include <TVectorD.h>
#include <TMatrixD.h>
#include <TMatrixDSym.h>
#include <TMatrixDSymEigen.h>
#include <TH1.h>
#include <TLegend.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TRandom3.h>

using namespace std;

////const char* resultsdir = "/uscms_data/d1/ilyao/KinematicFitterS11/ErrorScans/ToyValidation_results/";
////const char* mcSetprefix = "/uscms_data/d1/ilyao/DibosonS12/ErrorScans/1KMCSets/MCSet_";
//const char* mcSetprefix = "/uscms_data/d1/ilyao/DibosonS12/ErrorScans/1KMCSets_el/MCSet_";
//const char* mcNameprefix = "MCSet_";
const char* logprefix = "StandardFit_WpJDefault_Muon_";
const int NFloatPars=9;
const bool Verbose=true;
// //Locally
//const char* logDir = "/uscms_data/d1/ilyao/DibosonS12/ErrorScans/ToyValidationLocal_logs/";
//Via CONDOR:
const char* logDirectory = "./";




////Cross-Checks
// With fixed Factorization and Matching fractions (set float_fMUfSU=false)
const bool float_fMUfSU=true;
// With the Yields fixed to their expected values (& Poisson Smeared)
const bool fix_CentralYields=false;

////General Variables
double Lumi = 19200.0; //Luminosity in pb^-1
//double defWeights [] = {57.1097/9450414, 32.3161/10000267};
vector<double> defWeights;
// defWeights.push_back(57.1097/9450414);
// defWeights.push_back(32.3161/10000267);

//Diboson, Standard muon selection
const char* cuts_muStandard = "(((sqrt(JetPFCor_Pt[0]**2+JetPFCor_Pt[1]**2+2*JetPFCor_Pt[0]*JetPFCor_Pt[1]*cos(JetPFCor_Phi[0]-JetPFCor_Phi[1]))>70.)&&(abs(JetPFCor_Eta[0]-JetPFCor_Eta[1])<1.5)&&(abs(JetPFCor_dphiMET[0])>0.4)&&(W_mt>30.)&&(JetPFCor_Pt[1]>35.)&&(JetPFCor_Pt[0]>40.)&&(JetPFCor_Pt[2]<30.)&&((abs(JetPFCor_Eta[0])>2.4)||(JetPFCor_Pt[0]<30.)||(JetPFCor_bDiscriminatorCSV[0]<0.244))&&((abs(JetPFCor_Eta[1])>2.4)||(JetPFCor_Pt[1]<30.)||(JetPFCor_bDiscriminatorCSV[1]<0.244))&&((abs(JetPFCor_Eta[2])>2.4)||(JetPFCor_Pt[2]<30.)||(JetPFCor_bDiscriminatorCSV[2]<0.244))&&((abs(JetPFCor_Eta[3])>2.4)||(JetPFCor_Pt[3]<30.)||(JetPFCor_bDiscriminatorCSV[3]<0.244))&&((abs(JetPFCor_Eta[4])>2.4)||(JetPFCor_Pt[4]<30.)||(JetPFCor_bDiscriminatorCSV[4]<0.244))&&((abs(JetPFCor_Eta[5])>2.4)||(JetPFCor_Pt[5]<30.)||(JetPFCor_bDiscriminatorCSV[5]<0.244))&&(W_pt<200.)&&(vbf_event==0)&&(event_met_pfmet>25)&&(abs(W_muon_eta)<2.1)&&(W_muon_pt>25.))&&(Mass2j_PFCor>48.000)&&(Mass2j_PFCor<160.000))*effwt*puwt";



// -----------------------------------------------------------------------------------------------------------------------------------------------------------//

void GetCovMatrixAndCountsFromLog(const char* logfileName, TMatrixDSym & Cov, TVectorD& MeanPar, TVectorD& ErrPar, bool & isPositiveDefinite, const int NVars) {
  TString sPar1, sPar2, sPar3, sPar4;
  double dParV[NVars];
  char logline[2000];
  ifstream inLogFile(logfileName);
  isPositiveDefinite=true;
  int nSkip=-1;
  //cout << "4" << endl;
  while ( inLogFile.good() ) {
    inLogFile.getline(logline,2000);
    istrstream str(logline);
    //cout << "5" << endl;
    str >> sPar1 >> sPar2 >> sPar3 >> sPar4;

    ///Check the Convergence Status from HESSE
    if ( sPar3=="**HESSE" ) {
      for (Int_t dummy=0; dummy<3; dummy++) {
	inLogFile.getline(logline,2000);
	istrstream stra(logline);
	stra >> sPar1 >> sPar2 >> sPar3 >> sPar4;
	if ( dummy==1 ) {
	  if ( (sPar3=="CALCULATED")&&(sPar4=="SUCCESSFULLY") ) {
	    isPositiveDefinite=true;
	    //cout << "Covariance Matrix is Positive Definite" << endl;
	  } else {
	    isPositiveDefinite=false;
	    cout << "Covariance Matrix is NOT Positive Definite" << endl;
	  }
	}
	if ( sPar2=="ERROR:InputArguments" ) {
	  cout << "Skipping Error Messages" << endl;
	  bool skiperrors=true;
	  while (skiperrors) {
	    inLogFile.getline(logline,2000);
	    istrstream strb(logline);
	    strb >> sPar1 >> sPar2 >> sPar3 >> sPar4;
	    if ( sPar2!="ERROR:InputArguments" ) { skiperrors=false;}
	  }
	}
      }
      if ( isPositiveDefinite ) {
	istrstream str1(logline);
	str1 >> sPar1 >> sPar2 >> sPar3 >> sPar4;
	cout << " HESSE_Convergence = " << sPar4 << endl;
      }

      /// Get the Covariance Matrix
      nSkip=5;
      if ( !isPositiveDefinite ) { nSkip=4; }//no "COVARIANCE MATRIX CALCULATED SUCCESSFULLY" line
      for (Int_t j=0; j<nSkip+NVars; j++) {
	inLogFile.getline(logline,2000);
	istrstream strc(logline);
	strc >> sPar1 >> sPar2 >> sPar3 >> sPar4;
	if ( sPar1=="WARNING" ) {
	  cout << "Skipping WARNING - AT LIMIT message" << endl;
	  inLogFile.getline(logline,2000);
	}
      }
      for (Int_t j=0; j<NVars; j++) {
	inLogFile.getline(logline,2000);
	istrstream strd(logline);
	strd >> sPar1 >> sPar2;
	if ( (sPar1=="ELEMENTS")&&(sPar2=="ABOVE") ) {
	  inLogFile.getline(logline,2000);
	}
	istrstream str2(logline);
	//str2 >> dParV[0] >> dParV[1] >> dParV[2] >> dParV[3] >> dParV[4] >> dParV[5] >> dParV[6] >> dParV[7];
	for (Int_t k=0; k<NVars; k++) {
	  str2 >> dParV[k];
	  Cov(j,k)=dParV[k];
	}
      }
      /// Make sure the values below the diagonal are the ones kept (the ones above aren't always recorded in the log file)
      for (Int_t j=0; j<NVars; j++) {
	for (Int_t k=0; k<NVars; k++) {
	  if ( Cov(j,k)!=Cov(k,j) ) {
	    cout << "Symmetrizing the covariance matrix" << endl;
	    Cov(j,k)=Cov(k,j);
	  }
	}
      }
      /// Get the Mean Values
      nSkip=9;
      if ( !isPositiveDefinite ) { nSkip=11; }//skip extra two lines of "ERR MATRIX NOT POS-DEF" warnings
      for (Int_t j=0; j<NVars+nSkip; j++) {
	inLogFile.getline(logline,2000);
      }
      for (Int_t j=0; j<NVars; j++) {
	inLogFile.getline(logline,2000);
	istrstream str2(logline);
	str2 >> sPar1 >> dParV[1] >> sPar2 >> dParV[2];
	MeanPar(j)=dParV[1];
	ErrPar(j)=dParV[2];
	//cout << "Parameter=" << sPar1 << " FittedValue=" << dParV[1] << endl;
      }
    }

  }

}
// -----------------------------------------------------------------------------------------------------------------------------------------------------------//

void GetExpectedYieldsAndIndicesFromLog(const char* logfileName, double& Ndiboson, double& NWHbb, double& NWpJ, double& Ntop, double& NZpJ, double& NQCD, int& Idx_diboson, int& Idx_WHbb, int& Idx_WpJ, int& Idx_top, int& Idx_ZpJ, int& Idx_QCD, const int NVars) {
  TString sPar1, sPar2, sPar3, sPar4;
  double dPar1;
  char logline[2000];
  ifstream inLogFile(logfileName);
  while ( inLogFile.good() ) {
    inLogFile.getline(logline,2000);
    istrstream str(logline);
    //cout << "5" << endl;
    str >> sPar1 >> sPar2 >> sPar3 >> sPar4;

    ///Get the initial values
    if ( (sPar1=="load")&&(sPar2=="data") ) {
      inLogFile.getline(logline,2000);
      bool getadditionalyields=true;
      NWHbb=-1;
      NZpJ=-1;
      NQCD=-1;
      while ( getadditionalyields ) {
	///Parse the string beginning with 'RooRealVar::n_', either with or without skipping the string starting with 'explicitly'. Otherwise discontinue.
	inLogFile.getline(logline,2000);
	istrstream strA(logline);
	strA >> sPar1 >> sPar2 >> dPar1;
	if ( sPar1=="explicitly" ) {
     	  inLogFile.getline(logline,2000);
	  istrstream strB(logline);
	  strB >> sPar1 >> sPar2 >> dPar1;
	}
	if ( sPar1.Contains("n_diboson") ) { Ndiboson=dPar1; }
	if ( sPar1.Contains("n_WHbb") ) { NWHbb=dPar1; }
	if ( sPar1.Contains("n_top") ) { Ntop=dPar1; }
	if ( sPar1.Contains("n_WpJ") ) { NWpJ=dPar1; }
	if ( sPar1.Contains("n_ZpJ") ) { NZpJ=dPar1; }
	if ( sPar1.Contains("n_QCD") ) { NQCD=dPar1; }
	if ( !sPar1.Contains("RooRealVar::") ) { getadditionalyields=false; }
      }
    }
    ///Get the indices
    if ( (sPar1=="Floating")&&(sPar2=="Parameter") ) {
      Idx_WHbb=-1;
      Idx_ZpJ=-1;
      Idx_QCD=-1;
      inLogFile.getline(logline,2000);
      for (Int_t j=0; j<NVars; j++) {
	inLogFile.getline(logline,2000);
	istrstream str2(logline);
	str2 >> sPar1;
	if ( sPar1=="diboson_nrm" ) { Idx_diboson=j; }
	if ( sPar1=="WHbb_nrm" ) { Idx_WHbb=j; }
	if ( sPar1=="top_nrm" ) { Idx_top=j; }
	if ( sPar1=="WpJ_nrm" ) { Idx_WpJ=j; }
	if ( sPar1=="ZpJ_nrm" ) { Idx_ZpJ=j; }
	if ( sPar1=="QCD_nrm" ) { Idx_QCD=j; }
      }
    }
  }

}


// -----------------------------------------------------------------------------------------------------------------------------------------------------------//
void ComputeInitialYields(double& Ndiboson, double& NWHbb, double& NWpJ, double& Ntop, double& NZpJ, double& NQCD, const char* inputLogName, int InitRand=11, const int NVars = NFloatPars, bool includeNonPoissonFluctuations = true)
//// Combines the input yields in appropriate amounts (randomized according to a Poisson distribution) and saves them in a file of the form form mcSetprefix+leptonPrefix+#j_*.root with *=NStart-(NEnd+1),#=NJets
//// leptonPrefix = mu_, el_
//// btagPrefix = NoBtag_, WithBtag_
{
  TRandom PRand(InitRand);
  TRandom GRand(InitRand*137+2013);
  bool isCovMPositiveDefinite;
  int Idx_diboson,Idx_WHbb,Idx_WpJ,Idx_top,Idx_ZpJ,Idx_QCD;

  // char I_char[5];
  // double mjj, mjj_out;
  // int NJets=2;
  // const int WjIdx=6;
  // const int DibosonIdx=2;
  // TString L_str=leptonPrefix;
  // TString Tag_str=btagPrefix;
  // TString sourcefileSuffix_str=sourcefileSuffix;
  TMatrixDSym FullCov(NVars);//The Full Covariance Matrix 
  TVectorD Par(NVars);//Fitted Parameter Values
  TVectorD Err(NVars);//Their Errors (not used by this macro)
  GetCovMatrixAndCountsFromLog(inputLogName,FullCov,Par,Err,isCovMPositiveDefinite,NVars);
  // cout << "Unshifted NDiboson=" << Par(DibosonIdx) << endl;
  // Par(DibosonIdx)=(1.0+DibosonCentralYieldShift)*Par(DibosonIdx); 
  // cout << "Shifted NDiboson=" << Par(DibosonIdx) << endl;
  TMatrixDSymEigen CM(FullCov);
  if ( Verbose ) {
    if ( !isCovMPositiveDefinite ) {
      cout << "WARNING : Covariance Matrix Not Positive Definite" << endl;
    }
    cout << "Covariance Matrix = " << endl;
    for (Int_t j=0; j<NVars;j++) {
      for (Int_t k=0; k<NVars;k++) {
	cout << FullCov(j,k) << " ";
      }
      cout << endl;
    }

    cout << "Expected Variable Values = " << endl;
    for (Int_t j=0; j<NVars;j++) {
      cout << Par(j) << " ";
    }
    cout << endl;
  }
  TVectorD sigsqVal(NVars);
  sigsqVal = CM.GetEigenValues();
  TVectorD errVal(NVars);
  TMatrixD EVec = CM.GetEigenVectors();
  for (Int_t j=0; j<NVars;j++) {
    errVal(j)=pow(sigsqVal(j),0.5);
  }
  TVectorD RErr(NVars);//Errors (to be generated) in the Rotated System
  TVectorD UnRErr(NVars);//Errors in the physical coordinate system

  double VarVal[NVars];


  for (Int_t j=0; j<NVars;j++) {
  //   if ( !fix_CentralYields ) {
    RErr(j)=GRand.Gaus(0,errVal(j));
  //   } else {
  //     RErr(j)=0.0;
  //   }
  }
  UnRErr=EVec*RErr;
  for (Int_t j=0; j<NVars;j++) {
    VarVal[j]=Par(j)+UnRErr(j);
  }

  GetExpectedYieldsAndIndicesFromLog(inputLogName,Ndiboson,NWHbb,NWpJ,Ntop,NZpJ,NQCD,Idx_diboson,Idx_WHbb,Idx_WpJ,Idx_top,Idx_ZpJ,Idx_QCD,NVars);
  if ( Verbose ) {
    cout << "diboson: Nexpected=" << Ndiboson << " Idx=" << Idx_diboson << " initial fit_nrm=" << Par(Idx_diboson) << ", smeared fit_nrm=" << VarVal[Idx_diboson] << endl;
    cout << "WpJ: Nexpected=" << NWpJ << " Idx=" << Idx_WpJ << " initial fit_nrm=" << Par(Idx_WpJ) << ", smeared fit_nrm=" << VarVal[Idx_WpJ] << endl;
    cout << "top: Nexpected=" << Ntop << " Idx=" << Idx_top << " initial fit_nrm=" << Par(Idx_top)  << ", smeared fit_nrm=" << VarVal[Idx_top] << endl;
    if ( Idx_WHbb>=0 ) {
      cout << "WHbb: Nexpected=" << NWHbb << " Idx=" << Idx_WHbb << " initial fit_nrm=" << Par(Idx_WHbb)  << ", smeared fit_nrm=" << VarVal[Idx_WHbb] << endl;
    }
    if ( Idx_ZpJ>=0 ) {
      cout << "ZpJ: Nexpected=" << NZpJ << " Idx=" << Idx_ZpJ << " initial fit_nrm=" << Par(Idx_ZpJ)  << ", smeared fit_nrm=" << VarVal[Idx_ZpJ] << endl;
    }
    if ( Idx_QCD>=0 ) {
    cout << "QCD: Nexpected=" << NQCD << " Idx=" << Idx_QCD << " initial fit_nrm=" << Par(Idx_QCD)  << ", smeared fit_nrm=" << VarVal[Idx_QCD] << endl;
    }
  }
  if ( includeNonPoissonFluctuations ) {
    Ndiboson=PRand.Poisson(VarVal[Idx_diboson]*Ndiboson);
    NWpJ=PRand.Poisson(VarVal[Idx_WpJ]*NWpJ);
    Ntop=PRand.Poisson(VarVal[Idx_top]*Ntop);
  } else {
    Ndiboson=PRand.Poisson(Par(Idx_diboson)*Ndiboson);
    NWpJ=PRand.Poisson(Par(Idx_WpJ)*NWpJ);
    Ntop=PRand.Poisson(Par(Idx_top)*Ntop);
  }
  if ( Idx_WHbb>=0 ) {
    if ( includeNonPoissonFluctuations ) {
      NWHbb=PRand.Poisson(VarVal[Idx_WHbb]*NWHbb);
    } else {
      NWHbb=PRand.Poisson(Par(Idx_WHbb)*NWHbb);
    }
  } else {
    NWHbb=-1;
  }
  if ( Idx_ZpJ>=0 ) {
    if ( includeNonPoissonFluctuations ) {
      NZpJ=PRand.Poisson(VarVal[Idx_ZpJ]*NZpJ);
    } else {
      NZpJ=PRand.Poisson(Par(Idx_ZpJ)*NZpJ);
    }
  } else {
    NZpJ=-1;
  }
  if ( Idx_QCD>=0 ) {
    if ( includeNonPoissonFluctuations ) {
      NQCD=PRand.Poisson(VarVal[Idx_QCD]*NQCD);
    } else {
      NQCD=PRand.Poisson(Par(Idx_QCD)*NQCD);
    }
  } else {
    NQCD=-1;
  }

  if ( Verbose ) {
    cout << "diboson: Ngen=" << Ndiboson << endl;
    cout << "WHbb: Ngen=" << NWHbb << endl;
    cout << "WpJ: Ngen=" << NWpJ << endl;
    cout << "top: Ngen=" << Ntop << endl;
    cout << "ZpJ: Ngen=" << NZpJ << endl;
    cout << "QCD: Ngen=" << NQCD << endl;
  }

}
// -----------------------------------------------------------------------------------------------------------------------------------------------------------//

bool logfileContains(const char* logfileName, const char* stringOfInterest) {
  bool thefileContains=false;
  char logline[2000];
  ifstream inLogFile(logfileName);
  while ( inLogFile.good() ) {
    inLogFile.getline(logline,2000);
    //istrstream str(logline);
    TString str(logline);
    //cout << "str = " << str << endl;

    if ( str.Contains(stringOfInterest) ) {
      thefileContains=true;
    }
  }

  return thefileContains;

}


// -----------------------------------------------------------------------------------------------------------------------------------------------------------//

void processLogs(int NLogs=1000, const char* processlogPrefix="/uscms_data/d3/ilyao/Diboson8TeV/ToyValidation_Boosted_mu_logs/BoostedFit_Muon_", const char* outFileName="/uscms_data/d3/ilyao/Diboson8TeV/ToyValidation_results/1KMCGenSetFit_BoostedMuon_LogSummary.root", const char* initfileName="/uscms_data/d3/ilyao/DibosonS12/ErrorScans/ToyValidation_fixedCentralYields_mu/results/InitParams_mu2jNoBtag.root", const int NVars = NFloatPars)
//// Processes the logs and produces an output file. Log files are of the form processlogPrefix*.log, where *=0-(NLogs-1)
{
  TString inLogFileName, sPar1, sPar2, sPar3, sPar4, sPar5;
  char logline[2000];
  char I_char[5];
  double dPar;
  //  double dParV[6];
  double numDiboson,errDiboson,numWHbb,errWHbb,numQCD,errQCD,numTop,errTop,numWjets,errWjets,numZjets,errZjets,numTotal,errTotal,chi2dof,chi2prob; 
  double initDiboson, initWHbb, initQCD, initTop, initWjets, initZjets, initTotal;
  double genDiboson, genWHbb, genQCD, genTop, genWjets, genZjets;
  int Idx_diboson,Idx_WHbb,Idx_WpJ,Idx_top,Idx_ZpJ,Idx_QCD;
  bool isCovMPositiveDefinite;
  //  double init_Diboson, init_QCD, init_Top, init_Wjets, init_Zjets, init_Total;

  int Convergence;
  TMatrixDSym FullCov(NVars);//The Full Covariance Matrix
  TVectorD Par(NVars);//Fitted Parameter Values
  TVectorD Err(NVars);//Their Errors
  //cout << "0" << endl;

//   TFile* initfile= new TFile(initfileName, "READ");
//   TTree* InitTree = (TTree*) initfile->Get("InitTree");
//   InitTree->SetBranchAddress("initDiboson",&init_Diboson);
//   InitTree->SetBranchAddress("initQCD",&init_QCD);
//   //  InitTree->SetBranchAddress("initSingleTop",&init_SingleTop);
//   InitTree->SetBranchAddress("initTop",&init_TTbar);
//   InitTree->SetBranchAddress("initWjets",&init_Wjets);
//   InitTree->SetBranchAddress("initZjets",&init_Zjets);
//   InitTree->SetBranchAddress("initTotal",&init_Total);
// //   InitTree->SetBranchAddress("initfMU",&init_fMU);
// //   InitTree->SetBranchAddress("initfSU",&init_fSU);


  TFile *outfile = new TFile(outFileName, "RECREATE");
  TTree *OutTree = new TTree("OutTree","OutTree");

  //  cout << "1" << endl;
  OutTree->Branch("initDiboson",&initDiboson,"initDiboson/D");
  OutTree->Branch("numDiboson",&numDiboson,"numDiboson/D");
  OutTree->Branch("errDiboson",&errDiboson,"errDiboson/D");
  OutTree->Branch("initQCD",&initQCD,"initQCD/D");
  OutTree->Branch("numQCD",&numQCD,"numQCD/D");
  OutTree->Branch("errQCD",&errQCD,"errQCD/D");
  OutTree->Branch("initTop",&initTop,"initTop/D");
  OutTree->Branch("numTop",&numTop,"numTop/D");
  OutTree->Branch("errTop",&errTop,"errTop/D");
  OutTree->Branch("initWHbb",&initWHbb,"initWHbb/D");
  OutTree->Branch("numWHbb",&numWHbb,"numWHbb/D");
  OutTree->Branch("errWHbb",&errWHbb,"errWHbb/D");
  OutTree->Branch("initWjets",&initWjets,"initWjets/D");
  OutTree->Branch("numWjets",&numWjets,"numWjets/D");
  OutTree->Branch("errWjets",&errWjets,"errWjets/D");
  OutTree->Branch("initZjets",&initZjets,"initZjets/D");
  OutTree->Branch("numZjets",&numZjets,"numZjets/D");
  OutTree->Branch("errZjets",&errZjets,"errZjets/D");
  OutTree->Branch("initTotal",&initTotal,"initTotal/D");
  OutTree->Branch("numTotal",&numTotal,"numTotal/D");
  OutTree->Branch("errTotal",&errTotal,"errTotal/D");
  //  OutTree->Branch("errTotalCov",&errTotalCov,"errTotalCov/D");//Sum of the covariance matrix elements.
  OutTree->Branch("chi2dof",&chi2dof,"chi2dof/D");
  OutTree->Branch("chi2prob",&chi2prob,"chi2prob/D");
//   OutTree->Branch("initfSU",&initfSU,"initfSU/D");
//   OutTree->Branch("numfSU",&numfSU,"numfSU/D");
//   OutTree->Branch("errfSU",&errfSU,"errfSU/D");
//   OutTree->Branch("initfMU",&initfMU,"initfMU/D");
//   OutTree->Branch("numfMU",&numfMU,"numfMU/D");
//   OutTree->Branch("errfMU",&errfMU,"errfMU/D");
  OutTree->Branch("Convergence",&Convergence,"Convergence/I");

  //  cout << "2" << endl;

  for (Int_t i=0; i<NLogs; i++) {
    cout << "Processing Log " << i << " : " << endl;
    inLogFileName=".log";
    sprintf(I_char,"%i",i);
    inLogFileName=I_char+inLogFileName;
    inLogFileName=processlogPrefix+inLogFileName;
    //  cout << "3" << endl;
    ifstream inLogFile(inLogFileName);
    //cout << "4" << endl;
    genDiboson=-1;
    genWHbb=-1;
    genWjets=-1;
    genTop=-1;
    genZjets=-1;
    genQCD=-1;

    while ( inLogFile.good() ) {
      inLogFile.getline(logline,2000);
      istrstream str(logline);
      //cout << "5" << endl;
      str >> sPar1 >> sPar2 >> sPar3 >> sPar4;

      ///Get the number of generated events
      if ( sPar3=="RooRealVar::" ) {
	str >> sPar5 >> dPar;
	if ( sPar4=="n_diboson" ) { genDiboson=dPar; }
	if ( sPar4=="n_WHbb" ) { genWHbb=dPar; }
	if ( sPar4=="n_WpJ" ) { genWjets=dPar; }
	if ( sPar4=="n_top" ) { genTop=dPar; }
	if ( sPar4=="n_ZpJ" ) { genZjets=dPar; }
	if ( sPar4=="n_QCD" ) { genQCD=dPar; }
      }

      ///Get the Convergence Status from MIGRAD and HESSE
      if ( (sPar1=="Status")&&(sPar2=":") ) {
	if ( (sPar3=="MIGRAD=0")&&(sPar4=="HESSE=0") ) {
	  Convergence=0;
	} else {
	  Convergence=1;
	  cout << "Fit " << i << " did not converge: " << sPar2 << ", " << sPar3 << endl;
	}
      }

      ///Get the Total Yield
      if ( (sPar1=="total")&&(sPar2=="expected:") ) {
	istrstream str1(logline);
	str1 >> sPar1 >> sPar2 >> dPar;
	numTotal=dPar;
	inLogFile.getline(logline,2000);
	istrstream str2(logline);
	str2 >> sPar1 >> sPar2 >> dPar;
	initTotal=dPar;
      }

      ///Get the chi2dof and chi2prob
      if ( (sPar1=="chi2:") ) {
	istrstream str1(logline);
	str1 >> sPar1 >> sPar2 >> sPar3 >> sPar4 >> sPar5 >> dPar;
	chi2dof=dPar;
	inLogFile.getline(logline,2000);
	istrstream str2(logline);
	str1 >> sPar1 >> sPar2 >> dPar;
	chi2prob=dPar;
      }

    }


    GetExpectedYieldsAndIndicesFromLog(inLogFileName,initDiboson,initWHbb,initWjets,initTop,initZjets,initQCD,Idx_diboson,Idx_WHbb,Idx_WpJ,Idx_top,Idx_ZpJ,Idx_QCD,NVars);
    GetCovMatrixAndCountsFromLog(inLogFileName,FullCov,Par,Err,isCovMPositiveDefinite,NVars);
    if ( (Convergence==0)&&(!isCovMPositiveDefinite) ) {
      Convergence=2;
      cout << "Fit " << i << " converged but did not generate a positive definite error matrix" << endl;
    }
    /// Record the yield values and errors
    numDiboson=Par(Idx_diboson)*initDiboson;
    numWjets=Par(Idx_WpJ)*initWjets;
    numTop=Par(Idx_top)*initTop;
    if ( Idx_WHbb>=0 ) {
      numWHbb=Par(Idx_WHbb)*initWHbb;
      errWHbb=Err(Idx_WHbb)*initWHbb;
    } else {
      numWHbb=-2;
      errWHbb=-10;
    }
    if ( Idx_ZpJ>=0 ) {
      numZjets=Par(Idx_ZpJ)*initZjets;
      errZjets=Err(Idx_ZpJ)*initZjets;
    } else {
      numZjets=-2;
      errZjets=-10;
    }
    if ( Idx_QCD>=0 ) {
      numQCD=Par(Idx_QCD)*initQCD;
    } else {
      numQCD=-2;
      errQCD=-10;
    }
    errDiboson=Err(Idx_diboson)*initDiboson;
    errWjets=Err(Idx_WpJ)*initWjets;
    errTop=Err(Idx_top)*initTop;

    ///make the arrays
    int Idxs[NVars];
    double inits[NVars];
    int maxIdx=0;
    // Record the relevant index values
    for (int j=0; j<NVars;j++) {
      if ( (j==Idx_diboson)||(j==Idx_WHbb)||(j==Idx_WpJ)||(j==Idx_ZpJ)||(j==Idx_top)||(j==Idx_QCD) ) {
	Idxs[maxIdx]=j;
	maxIdx++;
      }

      if (j==Idx_diboson) {
	inits[j]=initDiboson;
      }
      if (j==Idx_WHbb) {
	inits[j]=initWHbb;
      }
      if (j==Idx_WpJ) {
	inits[j]=initWjets;
      }
      if (j==Idx_top) {
	inits[j]=initTop;
      }
      if (j==Idx_ZpJ) {
	inits[j]=initZjets;
      }
      if (j==Idx_QCD) {
	inits[j]=initQCD;
      }

    }
    ///
    errTotal=0.0;
    for (Int_t j=0; j<maxIdx;j++) {
      for (Int_t k=0; k<maxIdx;k++) {
	errTotal=errTotal + FullCov(Idxs[j],Idxs[k])*inits[j]*inits[k];
      }
    }
    errTotal=pow(errTotal,0.5);
    //cout << "errTotal=" << errTotal << endl;

    initDiboson=genDiboson;
    initWHbb=genWHbb;
    initQCD=genQCD;
    initTop=genTop;
    initWjets=genWjets;
    initZjets=genZjets;
    cout << "Recorded diboson: generated = " << initDiboson << ", fitted = " << numDiboson << " +/- " << errDiboson << endl;
    cout << "Recorded Wjets: generated = " << initWjets << ", fitted = " << numWjets << " +/- " << errWjets << endl;
    OutTree->Fill();
  }

  outfile->Write();
  outfile->Close();

}



// -----------------------------------------------------------------------------------------------------------------------------------------------------------//
void runToyMCFits_Diboson8TeV(int NStart, int NEnd, const char* inputLogName = "Fit_DibosonStandard_mu.txt", int InitRand = 5637, const char* CmdStringHead = "python -i runDiboson8TevFit.py -j 2 -m Diboson8TeVConfig --runPdfGenToySim --genParamFiles DibosonStandardFitOutPars_mu.txt --noNull --nosig", const char* CmdStringArgs = "topDibosonParameters.txt dibosonDibosonParameters.txt ZpJDibosonParameters.txt WpJDibosonParameters.txt", const char* logDir=logDirectory, const char* logPrefix=logprefix, const int NVars = NFloatPars, bool fullySmearAroundCenter = true, bool skipConfigurationsWithNegativeYields = true ) {
  TString Command,Command_body,Command_logend,LogName,ConfigName,ParamfileName;
  char I_char[7];

  int PresetRand=-1;
  double Ndiboson=-1;
  double NWHbb=-1;
  double NWpJ=-1;
  double Ntop=-1;
  double NZpJ=-1;
  double NQCD=-1;
  bool regenInitialYields=true;

  for (Int_t i=NStart; i<(NEnd+1); i++) {
    //make the log file
    cout << "Running Toy Fit " << i << endl;
    sprintf(I_char,"%i",i);
    LogName=".log";
    LogName=I_char+LogName;
    LogName=logPrefix+LogName;
    LogName=logDir+LogName;
    ///Run The Fit
    Command_logend=" > "+LogName;
    //Command=CmdStringArgs+Command;
    Command_body=" ";
    sprintf(I_char,"%i",InitRand+i*2+89);
    Command_body=I_char+Command_body;
    Command_body=" --seed "+Command_body;

    PresetRand=InitRand+387+i*3;
    regenInitialYields=true;
    while ( regenInitialYields ) {
      ComputeInitialYields(Ndiboson,NWHbb,NWpJ,Ntop,NZpJ,NQCD,inputLogName,PresetRand,NVars,fullySmearAroundCenter);
      regenInitialYields=false;
      if ( ((Ndiboson<0)||(NWpJ<0)||(Ntop<0)||(NWHbb<=-2)||(NZpJ<=-2)||(NQCD<=-2))&&(skipConfigurationsWithNegativeYields) ) {regenInitialYields=true;}
    }
    if ( NWHbb>0.0 ) {
      sprintf(I_char,"%i",int(NWHbb+0.5));
      Command_body=I_char+Command_body;
      Command_body=" --extWHbb " + Command_body;
    }
    if ( NZpJ>0.0 ) {
      sprintf(I_char,"%i",int(NZpJ+0.5));
      Command_body=I_char+Command_body;
      Command_body=" --extZpJ " + Command_body;
    }
    if ( NQCD>0.0 ) {
      sprintf(I_char,"%i",int(NQCD+0.5));
      Command_body=I_char+Command_body;
      Command_body=" --extQCD " + Command_body;
    }
    sprintf(I_char,"%i",int(Ntop+0.5));
    Command_body=I_char+Command_body;
    Command_body=" --exttop " + Command_body;
    sprintf(I_char,"%i",int(NWpJ+0.5));
    Command_body=I_char+Command_body;
    Command_body=" --extWpJ " + Command_body;
    sprintf(I_char,"%i",int(Ndiboson+0.5));
    Command_body=I_char+Command_body;
    Command_body=" --extdiboson " + Command_body;
    // create a file with temporary fit output parameters
    Command_body=" --fitparfn TempFitPars.txt" + Command_body;

    //Combine to form the command string
    Command = CmdStringHead  + Command_body + CmdStringArgs + Command_logend;
    //Command = CmdStringHead  + Command;
    cout << "Command=" << Command << endl;
    system(Command);

    //Check if the fit produced a positive-definite covariance matrix and if not rerun it using the output parameters as a starting point
    cout << "Checking the status of the covariance matrix ... ";
    if ( logfileContains(LogName,"covariance matrix quality: Full matrix, but forced positive-definite") ) {
      cout << "not positive-definite. Rerun the fit." << endl;
      Command = "mv " + LogName + " " + LogName + "_unconverged";
      cout << "Command=" << Command << endl;
      system(Command);
      Command = "rm " + LogName;
      cout << "Command=" << Command << endl;
      system(Command);
      Command = CmdStringHead  + Command_body + "TempFitPars.txt" + Command_logend;
      cout << "Command=" << Command << endl;
      system(Command);
    } else {
      cout << "is positive-definite" << endl;
    }


  }

}

//////////////////////////////////////////////////////////////////////
/////******** Use for explicit MC Set Generation: ***********/////////
//////////////////////////////////////////////////////////////////////
// -----------------------------------------------------------------------------------------------------------------------------------------------------------//
//  // void fillHist(TH1F* &h, const char* TreeName, const char* VarName, int NBins, double VarMin, double VarMax, const char* AddRest, const char* infilename, bool dosumW2 = false)
// ////Fill in histogram h from the file infilename.
// {
  
//   TString Restrictions, histload_str;
//   char Max_char[30], Min_char[30];

//   /// Make a string defining the restrictions (Range + Additional Ones).
  
//   sprintf(Max_char,"%.2e",VarMax);
//   cout << "Max_char=" << Max_char << endl;
//   sprintf(Min_char,"%.2e",VarMin);
//   cout << "Min_char=" << Min_char << endl;
//   if (AddRest!=0) {
//     Restrictions = AddRest;
//     Restrictions = ")&&"+Restrictions;
//   } else {
//     Restrictions = ")";
//   }
//   Restrictions=Max_char+Restrictions;
//   Restrictions="<"+Restrictions;
//   Restrictions=VarName+Restrictions;
//   Restrictions=")&&("+Restrictions;
//   Restrictions=Min_char+Restrictions;
//   Restrictions=">"+Restrictions;
//   Restrictions=VarName+Restrictions;
//   Restrictions="("+Restrictions;
//   cout << "Restrictions=" << Restrictions << endl;
  
//   //Open the files
//   TFile* f;
//   TTree* InTree;
//   f = new TFile(infilename);
//   InTree = (TTree*)f->Get(TreeName);
  
//   //Fill the histogram
//   histload_str=">>htemp";
//   histload_str=VarName+histload_str;
//   cout << "hist input =" << histload_str << endl;
  
//   //hname="h";
//   //cout << "hname=" << hname << endl;
    
//   TH1F* htemp = new TH1F("htemp","htemp",NBins,VarMin,VarMax);
//   InTree->Draw(histload_str,Restrictions);
//   h = (TH1F*)htemp->Clone();
//   if ( dosumW2 ) {
//     h->Sumw2();
//   }

//   cout << "hentries=" << h->GetEntries() << endl; 
//   delete htemp;
  
// }

/* // -----------------------------------------------------------------------------------------------------------------------------------------------------------//
void createBkgTemplateHist(TH1F* &h_tot, const int NBins, double varMin, double varMax, const char* inputTemplateFileDir = "/eos/uscms/store/user/lnujj/DibosonFitPostMoriond2013/", const char* inputTemplateFileNames = "RD_mu_WW_CMSSW532.root RD_mu_WZ_CMSSW532.root", vector<double> inputTemplateFileWeights = defWeights, const char* treeName = "WJet", const char* varName = "Mass2j_PFCor", const char* cuts = cuts_muStandard) {
////Create a histogram corresponding to a particular background (to be subsequently used as a template for dataset generation).

  TString inFileName, tempName, sPar1, sPar2;
  double outVar;
  double outVarVal[NBins+1];
  double  binVal, binErr, binCenter, nBinEntries;
  ///Read the input files and create the histogram
  TH1F* h_total = new TH1F("h_total","h_total",NBins,varMin,varMax);
  h_total->Sumw2();
  bool addfile = true;
  istrstream str_Names(inputTemplateFileNames);
  //istrstream str_Weights(inputTemplateFileWeights);
  int index = 0;
  while ( addfile ) {
    sPar1="";
    str_Names >> sPar1;
    cout << "sPar1=" << sPar1 << endl;
    if ( sPar1=="" ) {
      addfile=false;
    } else {
      //str_Weights >> sPar2;
      //cout << "sPar2=" << sPar2 << endl;
      //      dPar=atof(sPar2);
      //dPar=inputTemplateFileWeights[index];
      inFileName=sPar1;
      inFileName=inputTemplateFileDir+inFileName;
      TFile* f_temp = TFile::Open(inFileName);
      TTree* InTree_temp = (TTree*)f_temp->Get(treeName);
      TH1F* h_temp= new TH1F("h_temp","h_temp",NBins,varMin,varMax);
      tempName=">>h_temp";
      tempName=varName+tempName;
      InTree_temp->Draw(tempName,cuts);
      h_temp->Sumw2();
      h_temp->Scale(inputTemplateFileWeights[index]*Lumi);
      cout << "Adding " << h_temp->GetEntries() << " events with a scaling of " << inputTemplateFileWeights[index]*Lumi << "; Nexpected=" << h_temp->Integral() << endl;
      h_total->Add(h_temp);
      index++;
      delete h_temp;
    }
  }
  double htot_Entries=h_total->GetEntries();
  double htot_Integral=h_total->Integral();
  cout << "htot_Entries=" << htot_Entries << " : TotalExpected=htot_Integral=" << htot_Integral << endl;
  h_total->SetTitle("Scaled Input Events");
  h_total->Draw();

  h_tot = (TH1F*)h_total->Clone();
  h_tot->Sumw2();
  delete h_total;

}


// -----------------------------------------------------------------------------------------------------------------------------------------------------------//
void createTreeFromHist(TTree & outTree, TH1F* h_total, double NEvtsToGen, const int NBins, double varMin, double varMax, int InitRand = 5637, const char* outSupersetFileName = "test.root", const char* inputTemplateFileDir = "/eos/uscms/store/user/lnujj/DibosonFitPostMoriond2013/", const char* inputTemplateFileNames = "RD_mu_WW_CMSSW532.root RD_mu_WZ_CMSSW532.root", vector<double> inputTemplateFileWeights = defWeights, const char* treeName = "WJet", const char* varName = "Mass2j_PFCor", const char* cuts = cuts_muStandard) {
////Create a superset directly from the histogram. Events in each bin are generated independently subject to gaussian fluctuations (with the mean and sigma taken from the input template file).

  TRandom GRand(InitRand*128+2013);
  TString inFileName, tempName, sPar1, sPar2;
  //  float dPar;
  double outVar;
  double outVarVal[NBins+1];
  double  binVal, binErr, binCenter, nBinEntries;
  ///Read the input files and create the histogram
  TH1F* h_total = new TH1F("h_total","h_total",NBins,varMin,varMax);
  h_total->Sumw2();
  bool addfile = true;
  istrstream str_Names(inputTemplateFileNames);
  //istrstream str_Weights(inputTemplateFileWeights);
  int index = 0;
  while ( addfile ) {
    sPar1="";
    str_Names >> sPar1;
    cout << "sPar1=" << sPar1 << endl;
    if ( sPar1=="" ) {
      addfile=false;
    } else {
      //str_Weights >> sPar2;
      //cout << "sPar2=" << sPar2 << endl;
      //      dPar=atof(sPar2);
      //dPar=inputTemplateFileWeights[index];
      inFileName=sPar1;
      inFileName=inputTemplateFileDir+inFileName;
      TFile* f_temp = TFile::Open(inFileName);
      TTree* InTree_temp = (TTree*)f_temp->Get(treeName);
      TH1F* h_temp= new TH1F("h_temp","h_temp",NBins,varMin,varMax);
      tempName=">>h_temp";
      tempName=varName+tempName;
      InTree_temp->Draw(tempName,cuts);
      h_temp->Sumw2();
      h_temp->Scale(inputTemplateFileWeights[index]*Lumi);
      cout << "Adding " << h_temp->GetEntries() << " events with a scaling of " << inputTemplateFileWeights[index]*Lumi << "; Nexpected=" << h_temp->Integral() << endl;
      h_total->Add(h_temp);
      index++;
      delete h_temp;
    }
  }
  double htot_Entries=h_total->GetEntries();
  double htot_Integral=h_total->Integral();
  cout << "htot_Entries=" << htot_Entries << " : TotalExpected=htot_Integral=" << htot_Integral << endl;
  h_total->SetTitle("Scaled Input Events");
  h_total->Draw();

  //Fill the output tree
//   TTree *orderedTree = new TTree("orderedTree","orderedTree");//An 'unrandomized' tree where the entries are ordered based on their x values
//   orderedTree->Branch("oV",&orderedVar,"oV/D");

//   TFile *outFile = new TFile(outSupersetFileName, "RECREATE");
//   TTree *outTree = new TTree(treeName,treeName);
  TString outVarLabel="/D";
  outVarLabel=varName+outVarLabel;
  outTree->Branch(varName,&outVar,outVarLabel);
  //Generate bin by bin

  double nTotOutVar = 0;
  for (Int_t n=1; n<(NBins+1);n++) {
    binVal=h_total->GetBinContent(n);
    binErr=h_total->GetBinError(n);
    outVarVal[n]=GRand.Gaus(binVal,binErr);
    //nBinEntries=NEvtsToGen*outVarVal/htot_Integral;
    nTotOutVar=nTotOutVar+oubVarVal[n];
  }

  for (Int_t n=1; n<(NBins+1);n++) {
    binCenter=h_total->GetBinCenter(n);
    nBinEntries=oubVarVal[n]/nTotOutVar;
    cout << "Bin " << n << " : x=" << binCenter << ", y=" << binVal << "+/-" << binErr << endl;
    cout << "randomized y outVarVal=" << outVarVal << ", generate nBinEntries=" << nBinEntries << endl;
    for (Int_t j=0; j<nBinEntries;j++) {
      outVar=binCenter;
      outTree->Fill();
    }


  cout << "Total events generated = " << nTotEntries << endl;
  outFile->Write();
  outFile->Close();

}

// -----------------------------------------------------------------------------------------------------------------------------------------------------------//
void createMultipleSupersetsFromHist(int NBins, int startSet=0, int endSet=2, const char* outDir="/uscms_data/d3/ilyao/Diboson8TeV/GenSupersets/") {
////Create a superset directly from the histogram. Events in each bin are generated independently subject to gaussian fluctuations (with the mean and sigma taken from the input template file).
  const int NProcesses = 3;
  //  const int MaxWeights = 10;
  TString bkgInputDir, selectionCuts, outFileName, outFileLabelPrefix;
  char I_char[10];
  int randInitial;
  double varMin, varMax;
  if ( endSet>NProcesses ) { cerr << "Error: endSet exceeds the number of processes" << endl; }

  TString bkgLbl[NProcesses];
  TString bkgFiles[NProcesses];
  vector < vector<double> > bkgWeights;
  int bkgKEvtsToGen[NProcesses];

  bkgLbl[0]="diboson";
  bkgLbl[1]="WpJ";
  bkgLbl[2]="top";
  bkgKEvtsToGen[0]=10;
  bkgKEvtsToGen[1]=140000;
  bkgKEvtsToGen[2]=5000;

  bkgInputDir = "/eos/uscms/store/user/lnujj/DibosonFitPostMoriond2013/";
  bkgFiles[0]="RD_mu_WW_CMSSW532.root RD_mu_WZ_CMSSW532.root";
  bkgFiles[1]="RD_mu_W2Jets_CMSSW532.root RD_mu_W3Jets_CMSSW532.root RD_mu_W4Jets_CMSSW532.root RD_mu_ZpJ_CMSSW532.root";
  bkgFiles[2]="RD_mu_TTbar_CMSSW532.root RD_mu_STopS_Tbar_CMSSW532.root RD_mu_STopS_T_CMSSW532.root RD_mu_STopT_Tbar_CMSSW532.root RD_mu_STopT_T_CMSSW532.root RD_mu_STopTW_Tbar_CMSSW532.root RD_mu_STopTW_T_CMSSW532.root";

  vector<double> dibosonWts;
  dibosonWts.push_back(57.1097/9450414);
  dibosonWts.push_back(32.3161/10000267);
  bkgWeights.push_back(dibosonWts);

  vector<double> WpJWts;
  WpJWts.push_back(1750.0*1.16/33004921);
  WpJWts.push_back(519.0*1.16/15059503);
  WpJWts.push_back(214.0*1.16/12842803);
  WpJWts.push_back(3503.71/30209426);
  bkgWeights.push_back(WpJWts);

  vector<double> topWts;
  topWts.push_back(225.197/6893735);
  topWts.push_back(1.75776/139974);
  topWts.push_back(3.89394/259960);
  topWts.push_back(30.0042/1935066);
  topWts.push_back(55.531/3758221);
  topWts.push_back(11.1773/493458);
  topWts.push_back(11.1773/497657);
  bkgWeights.push_back(topWts);

  //  bkgWeights[0][MaxWeights]={57.1097/9450414, 32.3161/10000267};
  //  double bkgWeights[1][MaxWeights]={1750.0*1.16/33004921, 519.0*1.16/15059503, 214.0*1.16/12842803, 3503.71/30209426};
//   bkgWeights[][2]={225.197/6893735, 1.75776/139974, 3.89394/259960, 30.0042/1935066, 55.531/3758221, 11.1773/493458, 11.1773/497657};

  selectionCuts = cuts_muStandard;
  outFileLabelPrefix = "GenSuperset_muStdCuts_";

  randInitial=38801;
  varMin=48.0;
  varMax=160.0;

  for (Int_t i=startSet; i<(endSet+1);i++) {
    cout << "Generateing the superset for the " << bkgLbl[i] << " processes" << endl; 
    outFileName="KEvtsGen.root";
    sprintf(I_char,"%i",bkgKEvtsToGen[i]);
    outFileName=I_char+outFileName;
    outFileName=bkgLbl[i]+outFileName;
    outFileName="Bins_"+outFileName;
    sprintf(I_char,"%i",NBins);
    outFileName=I_char+outFileName;
    outFileName=outFileLabelPrefix+outFileName;
    outFileName=outDir+outFileName;
    cout << "outFileName=" << outFileName << endl;
    cout << "bkgWeights=" << bkgWeights[i][0] << ", " <<  bkgWeights[i][1] << ", ..." << endl;
    createSupersetFromHist(bkgKEvtsToGen[i]*1000,NBins,varMin,varMax,randInitial-129+i*2,outFileName,bkgInputDir,bkgFiles[i],bkgWeights[i],"WJet","Mass2j_PFCor",selectionCuts);
  }

}

// // -----------------------------------------------------------------------------------------------------------------------------------------------------------//

// void createIndividualTestDatasets(const char* inFitLogName = "Fit_DibosonStandard_mu.txt", int NStart=0, int NEnd=999, const char* outFileName = "/uscms_data/d3/ilyao/Diboson8TeV/GenSets/DibosonStandard_muGen_", int InitRand=57332, const char* outInitFileName="/uscms_data/d3/ilyao/DibosonS12/ErrorScans/ToyValidation_el/results/InitParams_el2jNoBtag.root", const int NVars = NFloatPars, bool fullySmearAroundCenter = true, bool skipConfigurationsWithNegativeYields = true)
// //// Combines the input yields in appropriate amounts (randomized according to a Poisson distribution) and saves them in the corresponding files, of the form outFileName*.root with *=NStart-(NEnd+1).
// /// The information from generation (i.e. initial yields) is stored in the outInitFileName
// {
//   TRandom PRand(InitRand);
//   TRandom GRand(InitRand*137-2012);
//   char I_char[5];
//   double mjj, mjj_out;
// //   int NJets=2;
// //   const int WjIdx=6;
// //   const int DibosonIdx=2;
// //   TString L_str=leptonPrefix;
// //   TString Tag_str=btagPrefix;
// //   TString sourcefileSuffix_str=sourcefileSuffix;
//   TMatrixDSym FullCov(NProcesses);//The Full Covariance Matrix 
//   TVectorD Par(NProcesses);//Yields and fractions
//   GetCovMatrixAndCountsFromLog(inFitLogName,FullCov,Par);
// //   cout << "Unshifted NDiboson=" << Par(DibosonIdx) << endl;
// //   Par(DibosonIdx)=(1.0+DibosonCentralYieldShift)*Par(DibosonIdx); 
// //   cout << "Shifted NDiboson=" << Par(DibosonIdx) << endl;
//   TMatrixDSymEigen CM(FullCov);
//   double covErr=0.0;
//   cout << "Covariance Matrix = " << endl;
//   for (Int_t j=2; j<NProcesses;j++) {
//     for (Int_t k=2; k<NProcesses;k++) {
//       cout << FullCov(j,k) << " ";
//       covErr=covErr+FullCov(j,k);
//     }
//     cout << endl;
//   }
//   cout << "Total Error=" << pow(covErr,0.5) << endl;

//   cout << "Expected Yields = " << endl;
//   for (Int_t j=0; j<NProcesses;j++) {
//       cout << Par(j) << " ";
//   }
//   cout << endl;

//   TVectorD sigsqVal(NProcesses);
//   sigsqVal = CM.GetEigenValues();
//   TVectorD errVal(NProcesses);
//   TMatrixD EVec = CM.GetEigenVectors();
//   for (Int_t j=0; j<NProcesses;j++) {
//     errVal(j)=pow(sigsqVal(j),0.5);
//   }
//   TVectorD RErr(NProcesses);//Errors (to be generated) in the Rotated System
//   TVectorD UnRErr(NProcesses);//Errors in the physical coordinate system

//   Int_t Nevt[NProcesses+2];
//   Int_t Nentry[NProcesses+2];
//   for (Int_t j=0; j<(NProcesses+2);j++) {
//     Nentry[j]=0;
//   }

//   double NevtWpJTot_All=0;
//   double NevtTot_All=0;
//   double NevtWpJTot_evt, NevtTot_evt, initDiboson, initQCD, initSingleTop, initTTbar, initZjets, initfMU, initfSU;


//   //Create a file which stores the initial Yields & Fractions for each dataset
//   TFile *outInitFile = new TFile(outInitFileName, "RECREATE");
//   TTree *InitTree = new TTree("InitTree","InitTree");

//   InitTree->Branch("initDiboson",&initDiboson,"initDiboson/D");
//   InitTree->Branch("initQCD",&initQCD,"initQCD/D");
//   InitTree->Branch("initSingleTop",&initSingleTop,"initSingleTop/D");
//   InitTree->Branch("initTTbar",&initTTbar,"initTTbar/D");
//   InitTree->Branch("initWjets",&NevtWpJTot_evt,"initWjets/D");
//   InitTree->Branch("initZjets",&initZjets,"initZjets/D");
//   InitTree->Branch("initTotal",&NevtTot_evt,"initTotal/D");


//   const int NProcesses;
//   TFile *f[NProcesses];
//   TTree *InTree[NProcesses];
//   TString sourcedir_str="/uscms_data/d3/ilyao/Diboson8TeV/GenSupersets/";

//   f[0]= new TFile(sourcedir_str + "GenSuperset_muStdCuts_100Bins_diboson5000KEvtsGen.root", "READ");//diboson
//   f[1]= new TFile(sourcedir_str + "GenSuperset_muStdCuts_100Bins_WpJ140000KEvtsGen.root", "READ");//W+Jets
//   f[2]= new TFile(sourcedir_str + "GenSuperset_muStdCuts_100Bins_top5000KEvtsGen.root", "READ");//top


//   for (Int_t j=0; j<NProcesses; j++) {
//     InTree[j] = (TTree*)f[j]->Get("WJet");
//     InTree[j]->SetBranchAddress("Mass2j_PFCor",&mjj);
//   }


//   TString outfilename;


//   int PresetRand=-1;
//   double Ndiboson=-1;
//   double NWpJ=-1;
//   double Ntop=-1;
//   double NZpJ=-1;
//   double NQCD=-1;
//   bool regenInitialYields=true;

//   for (Int_t i=NStart; i<(NEnd+1); i++) {
//     //make the log file
//     cout << "Generating File " << i << endl;
//     sprintf(I_char,"%i",i);
//     LogName=".log";
//     LogName=I_char+LogName;
//     LogName=logPrefix+LogName;
//     LogName=logDir+LogName;
//     ///Run The Fit
//     Command=" > "+LogName;
//     Command=CmdStringArgs+Command;
//     Command=" "+Command;
//     sprintf(I_char,"%i",InitRand+i*2+89);
//     Command=I_char+Command;
//     Command=" --seed "+Command;

//     PresetRand=InitRand+387+i*3;
//     regenInitialYields=true;
//     while ( regenInitialYields ) {
//       ComputeInitialYields(Ndiboson,NWpJ,Ntop,NZpJ,NQCD,inputLogName,PresetRand,NVars,fullySmearAroundCenter);
//       regenInitialYields=false;
//       if ( ((Ndiboson<0)||(NWpJ<0)||(Ntop<0)||(NZpJ<=-2)||(NQCD<=-2))&&(skipConfigurationsWithNegativeYields) ) {regenInitialYields=true;}
//     }



    

//     /// Fill the tree with the corresponding events
//     for (Int_t j=0; j<NProcesses;j++) {
//       //cout << "j=" << j << " Nevt=" << Nevt[j] << endl;
//       if ( (Nevt[j]<0)&&((j==0)||(j==1)) ) {
// 	//cout << "Nevt[NProcesses+j]=" << Nevt[NProcesses+j] << endl;
// 	//If we need to use MatchingDown or ScaleDown events:
// 	for (Int_t i=Nentry[NProcesses+j]; i<(Nentry[NProcesses+j]+Nevt[NProcesses+j]);i++) {
// 	  InTree[NProcesses+j]->GetEntry(i);
// 	  mjj_out=mjj;
// 	  TMCTree->Fill();
// 	}
// 	//cout << "j=" << j << " : wrote " << Nevt[NProcesses+j] << " events" << endl;
// 	Nentry[NProcesses+j]=Nentry[NProcesses+j]+Nevt[NProcesses+j];
//       } else {
// 	for (Int_t i=Nentry[j]; i<(Nentry[j]+Nevt[j]);i++) {
// 	  InTree[j]->GetEntry(i);
// 	  mjj_out=mjj;
// 	  TMCTree->Fill();
// 	}
// 	Nentry[j]=Nentry[j]+Nevt[j];
// 	//cout << "j=" << j << " : wrote " << Nevt[j] << " events" << endl;
//       }
//     }

//     fout->Write();
//     fout->Close();

//     initDiboson=Nevt[2];
//     initQCD=Nevt[3];
//     initSingleTop=Nevt[4];
//     initTTbar=Nevt[5];
//     initZjets=Nevt[7];
//     initfMU=double(Nevt[0])/NevtWpJTot_evt;
//     initfSU=double(Nevt[1])/NevtWpJTot_evt;
//     InitTree->Fill();
//     cout << "Generated File " << k << "  Event Counts:" << endl;
//     //NevtTot_All=fabs(Nevt[0])+fabs(Nevt[1])+Nevt[2]+Nevt[3]+Nevt[4]+Nevt[5]+Nevt[6]+Nevt[7]+NevtTot_All;
//     cout << "NMU=" << Nevt[0] << " NSU=" << Nevt[1] << " NDiboson=" << Nevt[2] << " NQCD =" << Nevt[3] << " NSingleTop=" << Nevt[4] << " NTTbar=" << Nevt[5] << " NWjetsDef=" << Nevt[6] << " NZjets=" << Nevt[7] << " NTotal=" << NevtTot_evt << endl;

//   }

//   outInitFile->Write();
//   outInitFile->Close();
//   cout << "AverageNtot=" << NevtTot_All/(NEnd-NStart+1) << "  AverageWpJTot=" << NevtWpJTot_All/(NEnd-NStart+1) << endl;

// }
*/
