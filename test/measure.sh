#!/bin/bash
for col in 7 8 9 10 11 12 13
do
    sed -i 's/#define COL.*/#define COL '"${col}"'/g' ../src/common.h
    make -j5
    time ./AMOBA "$@"
done
