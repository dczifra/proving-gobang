#include "heuristic.h"
#include "common.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

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

void classic_twopad(std::vector<Line>& lines){
    lines = {
        {{0,0},{0,1},{0,2},{0,3}},
        {{1,0},{1,3}},
        {{0,1},{1,2}},
        {{0,2},{1,1}},
        {{COL-2,0},{COL-2,3}},
        {{COL-1, 0},{COL-1, 1},{COL-1, 2},{COL-1, 3}},
        {{COL-2, 1}, {COL-1, 2}},
        {{COL-2, 2}, {COL-1, 1}}
    };

    // === INNER LINES ===
    if(COL>=8) add_horizontal_lines(lines, {0,1,2,3}, {1,COL-8}, 7);
    // === SIDE LINES ===
    int length = 5;
    add_horizontal_lines(lines, {0,1,2,3}, {0,0}, length);
    add_horizontal_lines(lines, {0,1,2,3}, {COL-length, COL-length}, length);
    // === DIAGONAL LINES ===
    add_diagonal_lines(lines, {0, COL-1});
    // === VERTICAL LINES ===
    add_vertical_lines(lines, {1, COL-2});

    remove_duplicates(lines);
}

void Heuristic::read_forbidden_strategy(){
    // =====================================
    // TAKE CARE, LOT OF BURNED VARIABLE !!!
    // =====================================
    std::ifstream inp("../boards/forbidden1.txt");
    int N,x,y;
    int R,C;
    inp>>N>>R>>C;
    for(int i=0;i<N;i++){
        board_int act_board = 0, def_board = 0, action;
        int rowNum;
        inp >>rowNum;
        // === Read attacker fields ===
        for(int j=0;j<rowNum;j++){
            inp>>action;
            act_board |= (1ULL << action);
        }
        // === Read defender fields ===
        for(int j=0;j<rowNum;j++){
            inp>>action;
            def_board |= (1ULL << action);
        }

        side_strategy[act_board] = def_board;
        forbidden_fields_left |= act_board;
        // === Add symmetry ===
        def_board = def_board << (40ULL);
        side_strategy[act_board << (40ULL)] = def_board;
        forbidden_fields_right |= (act_board << (40ULL));
    }

    forbidden_all = forbidden_fields_left | forbidden_fields_right;
}

void read_lines_from_file(std::vector<Line>& lines){
    std::ifstream inp("../boards/cross_board_6common.txt");
    //std::ifstream inp("../boards/cross_board_easy.txt");
    //std::istream& inp= std::cin;
    while(1){
        std::string line;
        getline(inp, line);

        // === Stop condition ===
        if(line=="") break;
        else if(line[0] == '#' || line[0] == '/') continue;
        std::stringstream inp_line(line);

        Line act_line={};
        while(!inp_line.eof()){
            std::pair<int,int> coord;
            char split;

            inp_line>>coord.first>>coord.second>>split;
            act_line.push_back(coord);
        }
        lines.push_back(act_line);
    }
    remove_duplicates(lines);
}

void Heuristic::generate_lines(){
    //classical_board(lines);
    //zsolts_board(lines);
    read_lines_from_file(lines);
    if(0){
        std::vector<int> lefts = {1,2,3,6,7,8};
        std::vector<int> rights = {41,42,43,46,47,48};
        std::vector<int> inner = {6,7,8,41,42,43};
        for(auto f: lefts) forbidden_fields_left |= (1ULL << f);
        for(auto f: rights) forbidden_fields_right |= (1ULL << f);
        for(auto f: inner) forbidden_fields_inner |= (1ULL << f);

        forbidden_all = forbidden_fields_left | forbidden_fields_right;
    }
    //read_forbidden_strategy();
}



