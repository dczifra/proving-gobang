#pragma once

#include <array>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <numeric>
#include <iomanip>
#include <iostream>

#define ROW 4
#define COL 12
#define LINEINROW 7
#define LAYERNUM 7

#define board_int std::uint64_t
#define BITSIZE 64
#define FULL_BOARD 0x0000ffffffffffffULL

const int ACTION_SIZE = ROW*COL;
const int MAX_ROUND = ROW*COL;

template<class T>
using mtx = std::array<std::array<T,ROW>, COL> ;
using Line = std::vector<std::pair<int,int>>;

struct Board;
void display(const Board board, bool end);
void display(const mtx<int>& board, bool end);
void display(const board_int board, bool end);
void print_mtx(const std::array<float, ACTION_SIZE>& mtx, int prec=5);

template<class T>
void make_zero(mtx<T>& data);

unsigned int selectBit(const board_int v, unsigned int r);