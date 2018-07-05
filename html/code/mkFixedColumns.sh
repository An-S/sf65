#!/bin/bash

if [ $# -lt 2 ]
then 
    echo "Too less params."
    echo "Use exe width files..."
    
    exit 1
fi

nfiles=$#
width=$1
shift
rawfiles="$*"
files=$(sort <<< $rawfiles)

pr -s'|' -tw $(( $width*$nfiles )) -m $files


