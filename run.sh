#!/usr/bin/env bash

if [ $# -eq 1 ]; then
        #work=$1
        WORKDIR=~/alice/test/sim/$1
        if [ ! -e $WORKDIR ]; then
                mkdir $WORKDIR
        fi
else
        echo 'argument is directory path'
        exit 1
fi

cd $WORKDIR

 
set -x

NEV=100
BMIN=0.
BMAX=5.
o2-sim -n ${NEV} -g pythia8pp -m TPC --trigger external --configKeyValues "TriggerExternal.fileName=trigger_impactb_pythia8.macro;TriggerExternal.funcName=trigger_impactb_pythia8(${BMIN}, ${BMAX})"

o2-sim-digitizer-workflow -b -q --onlyDet TPC --configKeyValues "TPCEleParam.DigiMode=1"

if [ ! -e data ]; then
        mkdir data
fi

#root -l -b -q ~/alice/macro/readDigitsAndCommonModetest_C.so

#./../../macro/flp1/Cumulative_frequency_adc
#./../../macro/flp1/Cumulative_frequency_cm

#root -l -q ~/alice/macro/cm_graph.C
#root -l -q ~/alice/macro/adc_graph.C



