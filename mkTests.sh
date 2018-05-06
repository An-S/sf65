#!/bin/bash

dir=testfiles

echo "Processing files:"
echo "-----------------"

for f in $dir/*.txt
do
    echo $f
    f_base=`basename $f .txt`
    ./sf65 -du -ml $f $dir/"$f_base"_-du-ml.out 1> /dev/null
    ./sf65 -dl -mu $f $dir/"$f_base"_-dl-mu.out 1> /dev/null
    ./sf65 -dl -ml $f $dir/"$f_base"_-dl-ml.out 1> /dev/null
    ./sf65 -du -mu $f $dir/"$f_base"_-du-mu.out 1> /dev/null
    ./sf65 $f $dir/"$f_base"_def.out  1> /dev/null
    ./sf65 -du -ml -l0 $f $dir/"$f_base"_-du-ml-l0.out 1> /dev/null
    ./sf65 -du -ml -l1 $f $dir/"$f_base"_-du-ml-l1.out 1> /dev/null
    ./sf65 -du -ml -l2 $f $dir/"$f_base"_-du-ml-l2.out 1> /dev/null
    ./sf65 -du -ml -e0 $f $dir/"$f_base"_-du-ml-e0.out 1> /dev/null
    ./sf65 -du -ml -e1 $f $dir/"$f_base"_-du-ml-e1.out 1> /dev/null
    ./sf65 -du -ml -m12 $f $dir/"$f_base"_-du-ml-m16.out 1> /dev/null
    ./sf65 -du -ml -s1 $f $dir/"$f_base"_-du-ml-s1.out 1> /dev/null
    ./sf65 -du -ml -o16 $f $dir/"$f_base"_-du-ml-o16.out 1> /dev/null
    ./sf65 -du -ml -c40 $f $dir/"$f_base"_-du-ml-c40.out 1> /dev/null
    ./sf65 -du -ml -n2 $f $dir/"$f_base"_-dl-mu-n2.out 1> /dev/null
done

echo "-----------------"
echo "Checking files:"
echo "-----------------"

for f in $dir/*.out
do
    echo $f
    f_base=`basename $f .out`
    if [ -f $dir/"$f_base".expected ]; then
        rm -f $dir/"$f_base".diff
        
        df=$(diff -y $f $dir/"$f_base".expected)
        if [ "$df" ]; then
            echo $df > $dir/"$f_base".diff
        
            echo "test failed in $f!";
        fi
        
    else
        echo "$dir/$_f_base file with expected results missing !"
    fi 
    
done