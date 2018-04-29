#!/bin/bash

dir=testfiles
for f in $dir/*.txt
do
    echo $f
    f_base=`basename $f .txt`
    ./sf65 -du -ml $f $dir/"$f_base"_-du-ml
    ./sf65 -dl -mu $f $dir/"$f_base"_-dl-mu
    ./sf65 -dl -ml $f $dir/"$f_base"_-dl-ml
    ./sf65 -du -mu $f $dir/"$f_base"_-du-mu
    ./sf65 $f $dir/"$f_base"_def
    ./sf65 -du -ml -l0 $f $dir/"$f_base"_-du-ml-l0
    ./sf65 -du -ml -l1 $f $dir/"$f_base"_-du-ml-l1
    ./sf65 -du -ml -l2 $f $dir/"$f_base"_-du-ml-l2
    ./sf65 -du -ml -e0 $f $dir/"$f_base"_-du-ml-e0
    ./sf65 -du -ml -e1 $f $dir/"$f_base"_-du-ml-e1
    ./sf65 -du -ml -m12 $f $dir/"$f_base"_-du-ml-m16
    ./sf65 -du -ml -s1 $f $dir/"$f_base"_-du-ml-s1
    ./sf65 -du -ml -o16 $f $dir/"$f_base"_-du-ml-o16
    ./sf65 -du -ml -c40 $f $dir/"$f_base"_-du-ml-c40
    ./sf65 -du -ml -n2 $f $dir/"$f_base"_-dl-mu-n2
done
