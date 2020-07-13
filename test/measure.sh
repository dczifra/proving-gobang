#!/bin/bash
for col in 5 6 7 8 9
do
    sed -i 's/#define COL.*/#define COL '"${col}"'/g' ../src/common.h
    make
    time ./AMOBA
done
