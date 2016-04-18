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
    dotfile=${jsonfile%%.json}.dot
    pdffile=${jsonfile%%.json}.pdf
    $scriptDir/gprof2dot.py -f json $jsonfile > $dotfile
    dot -Tpdf $dotfile -o $pdffile
    echo "[SUCCESS] Generated $pdffile"
else
    echo "[ERROR] $jsonfile not found."
fi

