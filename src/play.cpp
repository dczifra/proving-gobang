#include "play.h"
#include "artic_point.h"

#include<sstream>
#include<fstream>

Play::Play(std::string filename, bool disproof){
    player = 1;
    choose_problem(board, player, disproof);

    // === Read Solution Tree ===
    read_solution(filename);
    printf("Proof/disproof tree size: %zu\n", tree.states.size());
    printf("Isommap size: %zu\n", isom_map.size());
    human_player = (tree.get_states(board)->pn == 0 ? -1:1);

    //build_tree();
}

NodeType Play::choose_problem(Board& board, int& player, bool disproof){
    if(disproof) board.move({0,1, ROW*COL-1}, player);
    //board.move({0,1,23}, player);

    return (player==1?OR:AND);
}

void Play::read_solution(std::string filename){
    std::ifstream file(filename.c_str());
    if(file.is_open()){
        std::cout<<"Processing file...\n";
    }
    else{
        std::cout<<"File not found: "<<filename<<std::endl;
    }

    std::string s;
    while(std::getline (file, s)){
        //std::cout<<s<<std::endl;
        unsigned int pn, dn;
        Board b;

        std::stringstream sstream(s);
        sstream>>b.white>>b.black>>b.node_type>>pn>>dn;
        if(tree.get_states(b)==nullptr){
            PNS::PNSNode* node = new PNS::PNSNode(b, -1, -1, -1,tree.heuristic);
            node->pn = pn;
            node->dn = dn;
            tree.add_board(b, node);

            std::vector<uint64_t> isom = tree.isom_machine.get_canonical_graph(b, tree.heuristic.all_linesinfo);
            if(isom_map.find(isom) == isom_map.end()){
                isom_map[isom] = node;
            }
            else{
                //printf("Origin %d\n", board.node_type);
                //display(isom_map[isom]->board, true);
                //printf("New: %d\n", b.node_type);
                //display(b, true);
                //tree.isom_machine.get_conversion(isom_map[isom]->board, b, tree.heuristic.all_linesinfo);
            }
        }
        else{
            //printf("Duplicated state:\n");
            //display(b, true);
        }


    }
}

int Play::move_human(){
    int row, col, act = -1;
    std::cin>>row>>col;
    act = col*ROW+row;

    while(!board.is_valid(act)){
        printf("Not valid action: %d\n", act);
        std::cin>>row>>col;
        act = col*ROW+row;
    }
    
    board.move(act, human_player);
    //tree.simplify_board(board, act, -1);

    return act;
}

void Play::build_node(Board b){
    PNS new_tree;

    //assert(tree.states.find(b) != tree.states.end());

    for(int i=0;i<ACTION_SIZE;i++){
        //display(state.first, true);
        if(!b.is_valid(i)) continue;
        
        Board next = new_tree.extend(tree.get_states(b), i, false);
        PNS::PNSNode* child = tree.get_states(next);
        if(child != nullptr){
            tree.get_states(b)->children[i] = child;
        }
    }
}

void Play::play_with_solution2(){
    int act;

    while(!tree.game_ended(board, act)){
        std::vector<int> color;
        act = -1;
        build_node(board);
        // === Human player can choose ===
        if(player == human_player ){
            int row, col;
            std::cin>>row>>col;
            act = col*ROW+row;
            board = tree.get_states(board)->children[act]->board;
        }
        else{
            // === Find the next child in Solution Tree ===
            if(board.node_type == OR) act = tree.get_min_children(tree.get_states(board), PN, true);
            else act = tree.get_min_children(tree.get_states(board), DN, true);

            if(act == ACTION_SIZE or act == -1){
                printf("Not found next step %d\n", act);
            }
            else
            {
                board = tree.get_states(board)->children[act]->board;
            }
        }
        color.push_back(act);

        player = get_player(board.node_type);
        printf("Action: %d pn: %d\n", act, tree.get_states(board)->pn);
        display(board, true, color);
        std::cout<<board.white<<" "<<board.black<<std::endl;
    }
    std::cout<<"END\n";
}