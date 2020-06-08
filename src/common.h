#pragma once

#include <array>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <numeric>
#include <iomanip>
#include <iostream>

#define ROW 4
#define COL 5
#define LINEINROW 7
#define LAYERNUM 7

#define BITSIZE 64
#define board_int std::uint64_t
unsigned long long set_full_board();

const unsigned long long FULL_BOARD = set_full_board();
const int ACTION_SIZE = ROW*COL;
const int MAX_ROUND = ROW*COL;

template<class T>
using mtx = std::array<std::array<T,ROW>, COL> ;
using Line = std::vector<std::pair<int,int>>;

struct Board;
void display(const Board board, bool end);
void display(const mtx<int>& board, bool end);
void display(const board_int board, bool end);

template<class T>
void make_zero(mtx<T>& data);

unsigned int selectBit(const board_int v, unsigned int r);

struct Line_info{
    unsigned int size;
    board_int line_board;
    std::vector<int> points;
};

// === Why cannot put it into the .cpp ??? ===
template <typename T>
void print_mtx(T& mtx, int prec=5){
    std::cout.precision(prec);
    std::cout << std::fixed;
    
    for(int j=0;j<ROW;j++){
        for(int i=0;i<COL;i++){
            std::cout<<mtx[i*ROW+j]<<" ";
        }
        std::cout<<"\n";
    }
}