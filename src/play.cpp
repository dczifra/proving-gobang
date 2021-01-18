#include "play.h"
#include "artic_point.h"

#include<sstream>
#include<fstream>

Play::Play(std::string filename, bool disproof, bool talky, Args* args_):talky(talky),tree(args_){
    player = 1;
    args = args_;
    Board board;
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
    //board.move({6},player);

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
        sstream>>b.white>>b.black>>b.node_type>>b.score_left>>b.score_right>>pn>>dn;
        if(tree.get_states(b)==nullptr){
            
            PNSNode* node = new PNSNode(b, args);
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

int get_action(int base_slot, Board board){
    board_int valids = board.get_valids_without_ondegree(PNS::heuristic.all_linesinfo);
    
    int slot =0;
    for(int i=0;i<ACTION_SIZE;i++){
        if(valids & (1ULL << i)){
            if(slot == base_slot) return i;
            else slot++;
        }
    }
    assert(0); // base_slot out of range
    return -1;
}

void Play::build_node(Board b){
    PNS new_tree(args);
    int slot = 0;
    
    board_int valids = b.get_valids_without_ondegree(PNS::heuristic.all_linesinfo);
    for(int i=0;i<ACTION_SIZE;i++){
        if(valids & (1ULL << i)){
            Board next = new_tree.extend(tree.get_states(b), i, slot, false);
            PNSNode* child = tree.get_states(next);
            if(child != nullptr){
                tree.get_states(b)->children[slot] = child;
            }
            slot++;
        }
    }
}

void Play::build_node2(PNSNode* base_node){
    PNS new_tree(args);
    PNSNode* node = new PNSNode(base_node->board, args);
    new_tree.extend_all(node, false);
    for(unsigned int i=0;i<node->children.size();i++){
        Node* child = node->children[i];
        if(child == nullptr) return; // Extends stops, if node is proven
        else if(child->is_inner()){
            std::cout<<"Child extended ("<<i<<")\n";
            tree.extend(base_node, get_action(i,base_node->get_board()),i, false);
        }
        else{
            PNSNode* orig_child = tree.get_states(child->get_board());
            if(orig_child != nullptr){
                base_node->children[i] = orig_child;
                base_node->children[i]->pn = orig_child->pn;
                base_node->children[i]->dn = orig_child->dn;
                //display(child->get_board(), true);
            }
            else{
                //std::cout<<"Child not found ("<<i<<")\n";
            }
        }     
    }
}

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

void Play::play_with_solution(){
    Board base_board;
    choose_problem(base_board, player, false, args); // TODO
    Node* act_node = new PNSNode(base_board, tree.args);
    int act = -1;
    while(act_node->is_inner() || !tree.game_ended(act_node->get_board())){
        std::cout<<"Childnum: "<<act_node->child_num<<std::endl;

        std::vector<int> color;
        act = -1;
        if(!act_node->is_inner()) build_node2((PNSNode*)act_node);
        // === Human player can choose ===
        if(player == human_player ){
            int row, col;
            std::cout<<"[RES]\n";
            if(act_node->is_inner()) printf("Special\n");

            std::cin>>row>>col;
            act = col*ROW+row;
            if(act_node->is_inner()) act_node = act_node->children[act];
            else act_node = act_node->children[get_index(act, act_node->get_board())];
        }
        else{
            // === Find the next child in Solution Tree ===
            if(act_node->type == OR) act = tree.get_min_children_index(act_node, PN);
            else act = tree.get_min_children_index(act_node, DN);

            if(act == ACTION_SIZE or act == -1){
                printf("Not found next step %d\n", act);
            }
            else
            {
                int true_act = get_action(act, act_node->get_board());
                act_node = act_node->children[act];
                act = true_act;
            }
        }
        color.push_back(act);

        if(act_node->is_inner()){
            std::cout<<"Inner node\n";
            player = get_player(act_node->type);
        }
        else{
            Board board(act_node->get_board());
            player = get_player(board.node_type);
            if(talky){
                printf("Action: %d pn: %d\n", act, (int)tree.get_states(board)->pn);
                display(board, true, color);
                std::cout<<"Scores: "<<board.score_left<<" "<<board.score_right<<std::endl;
            }
            //std::cout<<"[DIFF] "<<board.white<<" "<<board.black<<std::endl;
            //print_board(board.white);
            //print_board(board.black);
        }
        std::cout<<"next done\n";
    }
    std::cout<<"[END]\n";
}
