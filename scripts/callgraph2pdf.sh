#!/bin/bash
scriptDir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
if [ -z "$1" ]
then
    echo "[INFO] Usage: $0 [callgraph.json]"
    echo "[INFO] Using default name <callgraph.json> as the input file name";
    jsonfile="callgraph.json"
else
    jsonfile=$1
fi

if [ -f "$jsonfile" ]
then
    dotfileTop=${jsonfile%%.json}Top.dot
    dotfileAll=${jsonfile%%.json}All.dot
    pdffileTop=${jsonfile%%.json}Top.pdf
    pdffileAll=${jsonfile%%.json}All.pdf
    $scriptDir/gprof2dot.py -f json $jsonfile > $dotfileTop
    # --skew=0.01 
    $scriptDir/gprof2dot.py -f json $jsonfile -e0 -n0 > $dotfileAll
    dot -Tpdf $dotfileTop -o $pdffileTop
    dot -Tpdf $dotfileAll -o $pdffileAll
    echo "[SUCCESS] Generated $pdffileTop and $pdffileAll"
else
    echo "[ERROR] $jsonfile not found."
fi
