#include <iostream>
#include <vector>
#include <array>
#include <cmath>

#include "common.h"

#pragma once 
class Heuristic{
public:
    std::vector<Line> lines;
    mtx<std::vector<Line>> lines_per_field;
public:
    Heuristic(){
        generate_lines();
    }
    
    void generate_lines();
    template<class T>
    std::array<mtx<T>, LAYERNUM+1> get_layers(mtx<int>& board);
    template<class T>
    std::vector<T> get_flat_layers(mtx<int>& board);
};

template<class T>
std::array<mtx<T>, LAYERNUM+1> Heuristic::get_layers(mtx<int>& board){
    std::array<mtx<double>, LAYERNUM+1> layers;
    for(int i=0;i<layers.size();i++){
        make_zero<double>(layers[i]);
    }

    for(int i=0;i<lines.size();i++){
        bool enemyless = true;
        int emptynum = 0;
        for(int j=0;j<lines[i].size();j++){
            int y = lines[i][j].first, x = lines[i][j].second;
            if(board[y][x]==-1){
                enemyless=false;
                break;
            }
            else if(board[y][x]==0) emptynum++;
        }
        if(enemyless){
            for(int j=0;j<lines[i].size();j++){
                int y = lines[i][j].first, x = lines[i][j].second;
                if(board[y][x]==0){
                    layers[emptynum][y][x]++;
                    layers[0][y][x]+=std::pow(2.0, -emptynum);
                }
            }
        }
    }
    return layers;
}

template<class T>
std::vector<T> Heuristic::get_flat_layers(mtx<int>& board){
    // === Description ===
    // Returns the flattened input for the NN
    //     - board (COL*ROW)
    //     - heur mtx (COL*ROW)
    //     - heur layers (LAYERSNUM*COL_ROW)
    std::vector<T> layers(ROW*COL*(LAYERNUM+2), 0.0);
    for(int i=0;i<lines.size();i++){
        bool enemyless = true;
        int emptynum = 0;
        for(int j=0;j<lines[i].size();j++){
            int y = lines[i][j].first, x = lines[i][j].second;
            if(board[y][x]==-1){
                enemyless=false;
                break;
            }
            else if(board[y][x]==0) emptynum++;
        }
        if(enemyless){
            for(int j=0;j<lines[i].size();j++){
                int y = lines[i][j].first, x = lines[i][j].second;
                if(board[y][x]==0){
                    layers[(emptynum+1)*(ROW*COL)+(y*ROW+x)]+=1.0;
                    layers[(ROW*COL)+(y*ROW+x)]+=std::pow(2.0, -emptynum);
                }
            }
        }
    }

    for(int y=0;y<COL;y++){
        for(int x=0;x<ROW;x++){
            layers[y*ROW+x]=board[y][x];
        }
    }
    return layers;
}

int half(const int& a){ return (a%2 == 0) ? a/2 : (a+1)/2;}

void Heuristic::generate_lines(){
    for(int y=0;y<COL;y++){
        // === LINEINROW starting from y ===
        bool big = (y+LINEINROW<COL && y>0);
        bool small = (y+half(LINEINROW)==COL || y==0);
        if(small || big){
            for(int x = 0;x<ROW;x++){
                Line l;
                int length = small?half(LINEINROW):LINEINROW;
                for(int i=0;i<length;i++){
                    l.push_back({y+i,x});
                }
                lines.push_back(l);
            }
        }

        // === Diagonals ===
        if(y<=COL-ROW || y>=ROW-1){
            Line l1;
            Line l2;
            for(int x = 0;x<ROW;x++){
                if(y<=COL-ROW) l1.push_back({y+x,x});
                if(y>=ROW-1) l2.push_back({y-x,x});
            }
            if(y<=COL-ROW) lines.push_back(l1);
            if(y>=ROW-1) lines.push_back(l2);
        }

        // === Full columns ===
        Line l;
        for(int x = 0;x<ROW;x++){
            l.push_back({y,x});
        }
        lines.push_back(l);
    }

    // === Corners ===
    // ## 3 ## 
    lines.push_back({{2,0}, {1,1}, {0,2}});
    lines.push_back({{COL-3,0}, {COL-2,1}, {COL-1,2}});
    lines.push_back({{0,ROW-3}, {1,ROW-2}, {2,ROW-1}});
    lines.push_back({{COL-1,ROW-3}, {COL-2,ROW-2}, {COL-3,ROW-1}});

    // ## 2 ##
    lines.push_back({{1,0}, {0,1}});
    lines.push_back({{COL-2,0}, {COL-1,1}});
    
    for(int i=0;i<lines.size();i++){
        for(int j=0;j<lines[i].size();j++){
            int y = lines[i][j].first, x = lines[i][j].second;
            //std::cout<<"("<<y<<","<<x<<") ";
            lines_per_field[y][x].push_back(lines[i]);
        }
        //std::cout<<std::endl;
    }
}




