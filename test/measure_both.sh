#!/bin/bash
for col in 5 6 7 8 9 10 # 11 12 13
do
    sed -i 's/#define COL.*/#define COL '"${col}"'/g' ../src/common.h
    make -j5 > compile.out 2>&1
    echo "Proof: $col"
    time ./AMOBA 
    echo "Disproof: $col"
    time ./AMOBA --disproof
done

