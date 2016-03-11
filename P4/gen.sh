#!/bin/bash

filename=$(basename $1 .glsl)
./glc < $1 > $filename.bc
llvm-dis $filename.bc
