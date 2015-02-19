#!/bin/bash

if [ -z "$1" ]
then
    echo "[INFO] Usage: $0 [communication.dot]"
    echo "[INFO] Using default name <communication.dot> as the input file name";
    dotfile="communication.dot"
else
    dotfile=$1
fi

if [ -f "$dotfile" ]
then
    pdffile=${dotfile%%.dot}.pdf
    gvpr -c 'N[$.degree==0]{delete(0,$);}' $dotfile > temp.dot
    dot -Tpdf temp.dot -o $pdffile
    rm temp.dot
    echo "[SUCCESS] Generated $pdffile"
else
    echo "[ERROR] $dotfile not found."
fi

