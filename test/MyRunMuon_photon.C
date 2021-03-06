void MyRunMuon_photon(double myflag=20112250, bool isQCD=false, int runflag=0)
{
  gSystem->Load("libFWCoreFWLite.so");
  gSystem->Load("libPhysicsToolsUtilities.so");
  gSystem->Load("libPhysicsToolsKinFitter.so");
  gROOT->ProcessLine(".include ../../../");
  gROOT->ProcessLine(".L Resolution.cc+");
  gROOT->ProcessLine(".L ../src/METzCalculator.cc+");
  gROOT->ProcessLine(".L ../src/QGLikelihoodCalculator.C+");
  gROOT->ProcessLine(".L EffTableReader.cc+");
  gROOT->ProcessLine(".L EffTableLoader.cc+");
  gROOT->ProcessLine(".L ClassifierOut/TMVAClassification_WWA_nJ2_mu_BDT.class.C+");
  gROOT->ProcessLine(".L ClassifierOut/TMVAClassification_WWA_nJ2_mu_M_BDT.class.C+");
  gROOT->ProcessLine(".L kanamuon_photon.C+");
  gROOT->ProcessLine("kanamuon_photon runover");
  //Set true/false for isQCD
  char mycmd[500]; sprintf(mycmd,"runover.myana(%.d,%i,%i)",myflag, isQCD,runflag); cout << "running :: "<<mycmd << endl;
  gROOT->ProcessLine(mycmd);
}
