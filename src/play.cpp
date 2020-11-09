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
        var pn, dn;
        Board b;

        std::stringstream sstream(s);
        sstream>>b.white>>b.black>>b.node_type>>pn>>dn;
        if(tree.get_states(b)==nullptr){
            PNS::PNSNode* node = new PNS::PNSNode(b, tree.heuristic);
            node->pn = pn;
            node->dn = dn;
            tree.add_board(b, node);
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

    return act;
}

void Play::build_node(Board b){
    PNS new_tree;
    int slot = 0;
    
    board_int valids = b.get_valids_without_ondegree(PNS::heuristic.all_linesinfo);
    for(int i=0;i<ACTION_SIZE;i++){
        //display(state.first, true);
        if(valids & (1ULL << i)){
	  Board next = new_tree.extend(tree.get_states(b), i, slot, false);
	    PNS::PNSNode* child = tree.get_states(next);
	    if(child != nullptr){
              tree.get_states(b)->children[slot] = child;
	    }
	    slot++;
	}
    }
}

int get_index(int act, Board board){
    std::vector<int> indexes(ACTION_SIZE, -1);
    int slot =0;
    board_int valids = board.get_valids_without_ondegree(PNS::heuristic.all_linesinfo);
    for(int i=0;i<ACTION_SIZE;i++){
        if(valids & (1ULL << i)){
	    indexes[i]=slot;
	    slot++;
        }
    }
    return indexes[act];
}

void Play::play_with_solution2(){
    int act = -1;
    while(!tree.game_ended(board)){
        std::vector<int> color;
        act = -1;
        build_node(board);
        // === Human player can choose ===
        if(player == human_player ){
            int row, col;
            std::cin>>row>>col;
            act = col*ROW+row;
            board = tree.get_states(board)->children[get_index(act, board)]->board;
        }
        else{
            // === Find the next child in Solution Tree ===
            if(board.node_type == OR) act = tree.get_min_children_index(tree.get_states(board), PN);
            else act = tree.get_min_children_index(tree.get_states(board), DN);

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
        printf("Action: %d pn: %d\n", act, (int)tree.get_states(board)->pn);
        display(board, true, color);
        std::cout<<board.white<<" "<<board.black<<std::endl;
    }
    std::cout<<"END\n";
}
