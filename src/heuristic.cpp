#include "heuristic.h"
#include "common.h"

template<class T>
void make_zero(mtx<T>& data){
    for(int j=0;j<COL;j++){
        for(int k=0;k<ROW;k++){
            data[j][k]=0;
        }
    }
}

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


int Heuristic::getGameEnded(mtx<int>& board, int action, int round){
    bool free_line = true;
    for(auto& point: lines_per_action[action]){
        if(board[action/ROW][action%ROW]==-1){
            free_line=false;
            break;
        }
    }

    if(free_line) return 1;
    else return round<MAX_ROUND-1?0:-1;
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


void Heuristic::generate_compressed_lines(){
    //std::cout<<lines.size()<<std::endl;

    all_linesinfo.resize(lines.size());
    for(int i=0;i<lines.size();i++){
        // === Do the compressed board ===
        board_int board = 0;
        for(auto field: lines[i]){
            int y = field.first, x = field.second;
            board_int action= y*ROW+x;
            board |= (1ULL<<action);
        }
        all_linesinfo[i].line_board = board;
        all_linesinfo[i].size = lines[i].size();
        all_linesinfo[i].index = i;
                
        // === Append board for every field ===
        for(auto field: lines[i]){
            int y = field.first, x = field.second;
            all_linesinfo[i].points.push_back(y*ROW+x);
        }

        // === add for every field ===
        for(auto field: lines[i]){
            int y = field.first, x = field.second;
            linesinfo_per_field[y*ROW+x].push_back(all_linesinfo[i]);
        }
    }
}

void Heuristic::generate_lines(){
    for(int y=0;y<COL;y++){
        // === LINEINROW starting from y ===
        bool big = (y+LINEINROW<COL && y>0) && INNER_LINE;
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
    if(!DIFFERENT_CORNER){
        lines.push_back({{COL-2,0}, {COL-1,1}});
    }
    else{
        lines.push_back({{COL-1,2}, {COL-2,3}});
    }

    lines_per_action.resize(ROW*COL);
    for(int i=0;i<lines.size();i++){
        for(int j=0;j<lines[i].size();j++){
            int y = lines[i][j].first, x = lines[i][j].second;
            //std::cout<<"("<<y<<","<<x<<") ";
            lines_per_field[y][x].push_back(lines[i]);
            lines_per_action[y*ROW+x].push_back(lines[i]);
        }
        //std::cout<<std::endl;
    }
}




