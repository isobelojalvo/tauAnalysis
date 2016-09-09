#!/bin/bash

EXPECTED_ARGS=1
if [ $# -ne $EXPECTED_ARGS ]
then
  echo "Usage: $0 JOB_NAME"
fi

farmoutAnalysisJobs $1 \
  --input-files-per-job=1 \
  --job-generates-output-name \
  --infer-cmssw-path \
  --input-file-list=/afs/hep.wisc.edu/home/ncinko/private/CMSSW_8_0_10/src/RecoTauTag/tauAnalysis/test/files_SUSYggH_hMass.txt\
  --assume-input-files-exist \
  ./runMINIAODtwoprong.py  \
  'inputFiles=$inputFileNames' 'outputFile=$outputFileName'
