#!/bin/bash

dotf=$1
nName=$2

usage()
{
    echo -e " This script deletes a node and its edges from dot file (updates  the file)"
    echo "Usage:    `basename $0` dotFileName nodeName"
}

if [ $# -eq 0 ]
then
    usage;
    exit
fi

echo "Deleting the Node ($nName) and its Edges from file ($dotf)"
nNo=`awk '/'\ $nName\ '/ {print $1}' $dotf`

if [ "$nNo" == "" ];
then
    echo "Node ($nName) not found in file ($dotf)"
else
    echo "Node No $nNo";
    grep -v ${nNo} $dotf > temp
    mv temp $dotf
fi

