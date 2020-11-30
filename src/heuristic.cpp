#include "heuristic.h"
#include "common.h"

#include <algorithm>

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
            board_int action = y*ROW+x;
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

Line create_horizontal_line(int x, int y, int length){
    Line l;
    for(int i=0;i<length;i++){
        l.push_back({y+i,x});
    }
    return l;
}

std::vector<board_int> get_comp_lines(std::vector<Line>& lines){
    std::vector<board_int> comp_lines(lines.size());
    for(int i=0;i<lines.size();i++){
        // === Do the compressed board ===
        board_int board = 0;
        for(auto field: lines[i]){
            int y = field.first, x = field.second;
            board_int action = y*ROW+x;
            board |= (1ULL<<action);
        }
        comp_lines[i] = board;
    }
    return comp_lines;
}

bool is_duplicate(board_int board1, board_int board2){
    // stronger line should come first !!!
    board_int intersection = (board1 & board2);
    if(intersection == board1 || intersection == board2) return true;
    return false;
}

void classic_2corners(std::vector<Line>& lines){
    // O X O O X O
    // X O O O O X
    // O O O O O O
    // O O O O O O
    
    // ## 2 ##
    lines.push_back({{1,0}, {0,1}});
    if(!DIFFERENT_CORNER){
        lines.push_back({{COL-2,0}, {COL-1,1}});
    }
    else{
        lines.push_back({{COL-1,2}, {COL-2,3}});
    }
}

void replace_2lines_with_inner2lines(std::vector<Line>& lines){
    // O O X O O O O O X O O 
    // O X O O O O O O O X O
    // O X O O O O O O O X O
    // O O X O O O O O X O O
    if(COL >= 9){
        lines.push_back({{2,0}, {1,1}});
        lines.push_back({{7,0}, {8,1}});
        lines.push_back({{COL-2,2}, {COL-3,3}});
        lines.push_back({{COL-9,2}, {COL-8,3}});
    }
}

void many_threelines(std::vector<Line>& lines){
    // === LEFT CORNER ===
    lines.push_back({{0,0}, {1,1}, {2,2}});
    lines.push_back({{1,0}, {2,1}, {3,2}});
    lines.push_back({{2,0}, {3,1}, {4,2}});

    lines.push_back({{0,3}, {1,2}, {2,1}});
    lines.push_back({{1,3}, {2,2}, {3,1}});
    lines.push_back({{2,3}, {3,2}, {4,1}});

    // === RIGHT CORNER ===
    lines.push_back({{COL-5,1}, {COL-4,2}, {COL-3,3}});
    lines.push_back({{COL-4,1}, {COL-3,2}, {COL-2,3}});
    lines.push_back({{COL-3,1}, {COL-2,2}, {COL-1,3}});

    lines.push_back({{COL-3,2}, {COL-2,1}, {COL-1,0}});
    lines.push_back({{COL-4,2}, {COL-3,1}, {COL-2,0}});
    lines.push_back({{COL-5,2}, {COL-4,1}, {COL-3,0}});
}

void add_horizontal_lines(std::vector<Line>& lines, std::vector<int> rows,
                            std::pair<int,int> cols, int length){
    for(int y=cols.first;y<=cols.second;y++){
        // === LINEINROW starting from y ===
        for(int x: rows){
            Line l;
            for(int i=0;i<length;i++){
                l.push_back({y+i,x});
            }
            lines.push_back(l);
        }
    }
}

void add_side_lines(std::vector<Line>& lines, int row, int length){
    add_horizontal_lines(lines, {row}, {0,0}, length);
    add_horizontal_lines(lines, {row}, {COL-length, COL-length}, length);
}

void add_diagonal_lines(std::vector<Line>& lines, std::pair<int,int> cols){
    for(int y = cols.first; y<=cols.second; y++){
        Line l1;
        Line l2;
        for(int x = 0;x<ROW;x++){
            if(y<=COL-ROW) l1.push_back({y+x,x});
            if(y>=ROW-1) l2.push_back({y-x,x});
        }
        if(y<=COL-ROW) lines.push_back(l1);
        if(y>=ROW-1) lines.push_back(l2);
    }
}

void add_vertical_lines(std::vector<Line>& lines, std::pair<int,int> cols){
    for(int y = cols.first; y<=cols.second; y++){
        // === Full columns ===
        Line l;
        for(int x = 0;x<ROW;x++){
            l.push_back({y,x});
        }
        lines.push_back(l);
    }
}

void remove_duplicates(std::vector<Line>& lines){
    // === Remove duplicated lines ===
    std::vector<board_int> comp_lines = get_comp_lines(lines);
    std::vector<int> duplicates;
    for(int i=0;i<comp_lines.size();i++){
        for(int j=i+1;j<lines.size();j++){
            if(is_duplicate(comp_lines[i], comp_lines[j])){
                duplicates.push_back(j);
            }
        }
    }
    std::sort(duplicates.begin(), duplicates.end());
    for(int i=0;i<duplicates.size();i++){
        lines.erase(lines.begin()+duplicates[duplicates.size()-1-i]);
    }
}

void classical_board(std::vector<Line>& lines){
    // === Extras ===
    classic_2corners(lines);
    //replace_2lines_with_inner2lines(lines);
    //many_threelines(lines);

    // === INNER LINES ===
    add_horizontal_lines(lines, {0,1,2,3}, {1,COL-8}, 7);
    // === SIDE LINES ===
    add_side_lines(lines, 0, 4);
    add_side_lines(lines, 1, 7);
    add_side_lines(lines, 2, 4);
    add_side_lines(lines, 3, 4);
    // === DIAGONAL LINES ===
    add_diagonal_lines(lines, {0, COL-1});
    // === VERTICAL LINES ===
    add_vertical_lines(lines, {0, COL-1});

    // === Corners ===
    // ## 3 ## 
    if(!ONLY_4){
        lines.push_back({{2,0}, {1,1}, {0,2}});
        lines.push_back({{COL-3,0}, {COL-2,1}, {COL-1,2}});
        lines.push_back({{0,ROW-3}, {1,ROW-2}, {2,ROW-1}});
        lines.push_back({{COL-1,ROW-3}, {COL-2,ROW-2}, {COL-3,ROW-1}});
    }

    remove_duplicates(lines);
}

void zsolts_board(std::vector<Line>& lines){
    // === Extras ===
    lines.push_back({{1,0},{0, 1}, {COL-1, 0}});
    lines.push_back({{1,3},{0, 2}, {COL-1, 3}});
    //lines.push_back({{COL-1, 0}, {COL-1, 3}});

    lines.push_back({{COL-3, 0}, {COL-2, 1}, {COL-1, 1}});
    lines.push_back({{COL-3, 3}, {COL-2, 2}, {COL-1, 2}});
    //lines.push_back({{COL-1, 1}, {COL-1, 2}});

    // === Corners ===
    // ## 3 ## 
    if(!ONLY_4){
        lines.push_back({{2,0}, {1,1}, {0,2}});
        lines.push_back({{COL-4,0}, {COL-3,1}, {COL-2,2}});
        lines.push_back({{0,ROW-3}, {1,ROW-2}, {2,ROW-1}});
        lines.push_back({{COL-2,ROW-3}, {COL-3,ROW-2}, {COL-4,ROW-1}});
    }

    // === INNER LINES ===
    add_horizontal_lines(lines, {0,1,2,3}, {1,COL-9}, 7);
    // === SIDE LINES ===
    int length = 4;
    add_horizontal_lines(lines, {1}, {0,0}, length);
    add_horizontal_lines(lines, {1}, {COL-length-1, COL-length-1}, length);
    add_horizontal_lines(lines, {0,2,3}, {0,0}, 4);
    add_horizontal_lines(lines, {0,2,3}, {COL-4-1, COL-4-1}, 4);
    // === DIAGONAL LINES ===
    add_diagonal_lines(lines, {0, COL-2});
    // === VERTICAL LINES ===
    add_vertical_lines(lines, {0, COL-2});

    remove_duplicates(lines);
}

void Heuristic::generate_lines(){
    classical_board(lines);
    //zsolts_board(lines);
}



