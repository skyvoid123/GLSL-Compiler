#!/bin/bash
make clean
make
rightcount=0
totalcount=0
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
        ((rightcount+=1))
        ((totalcount+=1))
        echo
    else
        echo "$filename is incorrect"
        rm samples/$filename.diff
        rm samples/$filename.output
        ((totalcount+=1))
        echo
    fi
done
echo "Passed $rightcount out of $totalcount"
