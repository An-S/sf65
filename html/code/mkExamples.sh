#!/bin/bash
sf65="../../sf65"
if [ ! $# -eq 1 ]
then 
    echo "Use ./exe file"
    
    exit 1
fi

for w in $(seq 8 2 16) ;do
  $sf65 -du -ml -m$w $1 "${1%.*}-du-ml-m$w.s"
done


