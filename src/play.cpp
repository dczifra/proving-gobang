#include "play.h"
#include "artic_point.h"

#include<sstream>
#include<fstream>

Play::Play(std::string filename, bool disproof, bool talky, Args* args_):talky(talky),tree(args_){
    player = 1;
    args = args_;
    choose_problem(board, player, disproof, args);

    // === Read Solution Tree ===
    read_solution(filename);
    printf("Proof/disproof tree size: %zu\n", tree.states.size());
    printf("Isommap size: %zu\n", isom_map.size());
    if(board.node_type == OR){
        human_player = (tree.get_states(board)->pn == 0 ? -1:1);
    }
    else{
        human_player = (tree.get_states(board)->pn == 0 ? 1:-1);
    }

    //build_tree();
}

NodeType Play::choose_problem(Board& board, int& player, bool disproof, Args* args){
    if(disproof) board.move({0,1, ACTION_SIZE-1}, player);
    //board.move({1,4, 5, 2, ROW*COL-3, ROW*COL-8, ROW*COL-7, ROW*COL-2,}, player);
    //board.move({1,5, ROW*COL-7, ROW*COL-11}, player);

    if(args->START > -1) board.move({args->START}, player);

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
            
            PNS::PNSNode* node = new PNS::PNSNode(b, args);
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

void Play::build_node(Board b){
    PNS new_tree(args);
    int slot = 0;
    
    board_int valids = b.get_valids_without_ondegree(PNS::heuristic.all_linesinfo);
    for(int i=0;i<ACTION_SIZE;i++){
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
/*
void Play::build_licit_node(const Board& b, int action){
    PNS new_tree(args);
    PNS::PNSNode* node = new PNS::PNSNode(b, args);
    PNS::PNSNode* branch;
    new_tree.extend(node, action, get_index(action, b), false);
    int choice;
    // TODO: Add child1, child2
    // 1. Licit or neighbour step
    if(get_player(OR) == human_player){
        std::cout<<"0: licit\n1: neighbour\n";
        std::cin>>choice;
    }
    else{
        // If neighbour step in proof tree
        Board neigh = node->children[0]->children[1]->board;
        if(tree.get_states(neigh) != nullptr) choice = 1;
        else choice = 0;
    }
    branch = node->children[0]->children[choice];

    // 2. if licit
    if(choice == 0){
        // Choose licit
        if(get_player(AND) == human_player){
            printf("Choose licit: (MAX: %d)\n", (int)branch->children.size());
            std::cin>>choice;
        }
        else{
            choice = -1;
            for(int i=0;i<branch->children.size();i++){
                PNS::PNSNode *rej, *acc;
                rej = branch->children[0];
                acc = branch->children[1];
                if(tree.get_states(rej->board) != nullptr &&
                   tree.get_states(acc->board) != nullptr){
                    choice = i;
                    break;
                }
            }
            assert(choice != -1);
        }
        branch = node->children[choice];

        // Accept or reject licit
        if(get_player(OR) == human_player){
            printf("Reject: 0\nAccept: 1\n");
            std::cin>>choice;
        }
        else{
            if(tree.get_states(branch->children[0]->board) != nullptr){
                choice = 0;
            }
            else if(tree.get_states(branch->children[0]->board) != nullptr){
                choice = 1;
            }
            else assert(0);
        }
        branch = node->children[choice];
        board = branch->board;
    }
    else{
        board = branch->board;
    }
}
*/

void print_diff(board_int act_board, board_int last_board){
    for(int i=0;i<ACTION_SIZE;i++){
        board_int field = 1ULL << i;
        if((act_board & field ) && !(last_board & field)){
            std::cout<<i<<" ";
        }
    }
    std::cout<<std::endl;
}

void print_board(board_int board){
    for(int i=0;i<ACTION_SIZE;i++){
        if(board & (1ULL << i)) std::cout<<i<<" ";
    }
    std::cout<<std::endl;
}

void Play::play_with_solution2(){
    int act = -1;
    Board last_board;
    while(!tree.game_ended(board)){
        std::vector<int> color;
        //if((1ULL << act) & tree.heuristic.forbidden_all) build_licit_node(board, act);
        act = -1;
        build_node(board);
        // === Human player can choose ===
        if(player == human_player ){
            int row, col;
            std::cout<<"[RES]\n";
            std::cin>>row>>col;
            act = col*ROW+row;
            // TODO
            //std::cout<<tree.get_states(board)->heuristic_value<<std::endl;
            //if(tree.get_states(board)->heuristic_value == -1){
            //    std::cout<<"Licit node\n";
            //    board = tree.get_states(board)->children[act]->board;
            //}
            //else board = tree.get_states(board)->children[get_index(act, board)]->board;
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
                // TODO
                //board = tree.get_states(board)->children[act]->board;
            }
        }
        color.push_back(act);

        player = get_player(board.node_type);
        if(talky){
            printf("Action: %d pn: %d\n", act, (int)tree.get_states(board)->pn);
            display(board, true, color);
        }
        std::cout<<"[DIFF] "<<board.white<<" "<<board.black<<std::endl;
        //print_diff(board.white, last_board.white);print_diff(board.black, last_board.black);
        print_board(board.white);
        print_board(board.black);
        last_board = board;
    }
    std::cout<<"[END]\n";
}
