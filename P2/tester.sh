#!/bin/bash
make clean
make
for f in samples/*.glsl
do
    echo "Checking File $f"
    filename=$(basename $f .glsl)
    ./glc < $f &> samples/$filename.output
    if diff samples/$filename.output samples/$filename.out > samples/$filename.diff
    then
        echo "$filename is correct"
        rm samples/$filename.diff
        rm samples/$filename.output
        echo
    else
        echo "$filename is incorrect"
        rm samples/$filename.diff
        rm samples/$filename.output
        echo
    fi
done
