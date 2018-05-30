#!/bin/bash

for f in *.out; do
    f_base=`basename $f .out`
    if [ ! -f $f_base.expected ]; then
        echo "$f_base.expected missing"
    fi
done

for f in *.expected; do
    f_base=`basename $f .expected`
    if [ ! -f $f_base.out ]; then
        echo "$f_base.out missing"
    fi
done