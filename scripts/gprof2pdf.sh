#!/bin/bash

rootdir=`pwd`
gprofFile=$1
scriptDir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

filename=$(basename "$gprofFile")
extension="${filename##*.}"
filename="${filename%.*}"
pdfFile=${filename}.pdf
dotFile=${filename}.dot

# for all functions
#cat ${gprofFile} | gprof2dot.py -e0 -n0 --skew=0.01 | dot -Tpdf -o $pdfFile

# for some top contributing functions
#cat ${gprofFile} | gprof2dot.py --skew=0.01 | dot -Tpdf -o $pdfFile

$scriptDir/gprof2dot.py --skew=0.01 ${gprofFile} > $dotFile
dot -Tpdf -o $pdfFile $dotFile
