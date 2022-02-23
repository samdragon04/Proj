#!/bin/bash


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

#O2 simulation & digitizer
echo o2-sim start!
o2-sim -m TPC -n 10 -g pythia8pp  

echo o2-digitizer start!
o2-sim-digitizer-workflow --onlyDet TPC --interactionRate [5e4] -b -q --configKeyValues "TPCEleParam.DigiMode=1"

if [ ! -e data ]; then
	mkdir data
fi

#root -l -b -q ~/alice/test/macro/macroForMedian.C
