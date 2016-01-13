#!/bin/bash
for f in samples/*.frag
do
    echo "Checking File $f"
    filename=$(basename $f .frag)
    ./glc < $f > samples/$filename.output
    if cmp -s samples/$filename.output samples/$filename.out > samples/$filename.diff
    then
        echo "$filename is correct"
        echo 
        rm samples/$filename.diff
    else
        echo "$filename is incorrect check samples/$filename.diff"
        echo
    fi
done

for f in samples/*.glsl
do
    echo "Checking File $f"
    filename=$(basename $f .glsl)
    ./glc < $f > samples/$filename.output
    if cmp -s samples/$filename.output samples/$filename.out > samples/$filename.diff
    then
        echo "$filename is correct"
        rm samples/$filename.diff
        echo
    else
        echo "$filename is incorrect check samples/$filename.diff"
        echo
    fi
done
