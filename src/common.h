#pragma once

#define ROW 4
#define COL 12
#define LINEINROW 7
#define LAYERNUM 7

#include <array>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <numeric>
#include <iomanip>
#include <iostream>

struct Args{
    int simulationNum = 10000;
};


template<class T>
using mtx = std::array<std::array<T,ROW>, COL> ;
using Line = std::vector<std::pair<int,int>>;

template<class T>
void make_zero(mtx<T>& data){
    for(int j=0;j<COL;j++){
        for(int k=0;k<ROW;k++){
            data[j][k]=0;
        }
    }
}

template<class T>
void print_mtx(mtx<T>& data, int prec=5){
    std::cout.precision(prec);
    std::cout << std::fixed;
    
    for(int j=0;j<ROW;j++){
        for(int i=0;i<COL;i++){
            std::cout<<data[i][j]<<" ";
        }
        std::cout<<"\n";
    }
}


