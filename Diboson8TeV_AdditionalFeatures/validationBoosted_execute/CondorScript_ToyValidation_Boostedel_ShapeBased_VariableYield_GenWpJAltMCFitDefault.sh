#! /bin/sh
echo "Starting" 
pwd
##site_arch_choice=slc4_ia32_gcc345
##export BUILD_ARCH=`echo $site_arch_choice | awk -F_ '{print $1"_"$2}'`
##export SCRAM_ARCH=$site_arch_choice
export BUILD_ARCH=slc5_amd64_gcc462
export SCRAM_ARCH=slc5_amd64_gcc462
##setenv X509_USER_PROXY /uscms/home/aperloff/x509up_u43841
tar -xvf FullRelease5_3_15.tar.gz
##source /cvmfs/cms.cern.ch/cmsset_default.sh
##scram p CMSSW_5_3_2_patch4
mv *.* CMSSW_5_3_15_patch1/src/ElectroWeakAnalysis/VPlusJets/test/
cd CMSSW_5_3_15_patch1/src/ElectroWeakAnalysis/VPlusJets/test/
scramv1 b ProjectRename
scram b
pwd
##cd /uscms/home/ilyao/EWKVPLUSJETS/CMSSW_5_3_2_patch4/src/ElectroWeakAnalysis/VPlusJets/test
##source /uscmst1/prod/grid/gLite_SL5.csh
##source /uscmst1/prod/sw/cms/shrc prod
##pwd
##eval `scram runtime -csh`
eval `scram runtime -sh`
##cmsenv
##cd -
which root
##tar -xvf AllToyFitDiboson8TeV_GeneralFiles_NoLib.tar.gz
##source root/bin/thisroot.csh
##source root/bin/thisroot.sh
##which root
##ls

#### Define the process and the number of toy datasets to fit
Process="${BASH_ARGV[1]}"
NToys="${BASH_ARGV[0]}"
echo "Process=${Process}"
echo "NToys=${NToys}"
StartToy="$(( ${Process} * $NToys ))"
EndToy="$(( ${NToys} + ${StartToy} - 1))"
echo "StartToy=NToys*Process-1=$StartToy"
echo "EndToy=NToys+StartToy-1=$EndToy"
#### Run the Toy Fits
echo "electrons, Boosted. Fit the Alternate V+Jets parametrization generated sets with the default configuration"
initRand="$(( 4*${Process} + 2805671 ))"
echo "initRand=${initRand}"
CstrNoBtag="runToyMCFits.cc+(${StartToy},${EndToy},4,${initRand},\"Fit_DibosonBoosted_el_log.txt\",\"python -i runDibosonBoosted8TeVFit.py -j 2 --electron -m DibosonBoosted8TeVConfig --runPdfGenToySim --useAlternateModelsToGen --genParamFile MCBoostedParametersWpJAlt_el.txt --noNull --nosig\",\"topDibosonBoostedParameters_SidebandModified_el.txt dibosonDibosonBoostedParameters_SidebandModified_el.txt WpJDibosonBoostedParameters_el.txt\",\"./\",\"BoostedFitElectron_ShapeBased_VariableYield_GenWpJAltMCFitDefault_\",true,true)"
echo "CstrNoBtag=$CstrNoBtag"
eval "root -l -b -q '$CstrNoBtag'"
mv *.log ../../../../../
pwd