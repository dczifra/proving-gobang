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
        sstream>>b.white>>b.black>>b.node_type>>pn>>dn;
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
            tree.extend(base_node, i, get_index(i,child->get_board()), false);
        }
        else{
            PNSNode* orig_child = tree.get_states(child->get_board());
            if(orig_child != nullptr){
                //tree.extend(base_node, i, get_index(i,child->get_board()), false);
                base_node->children[i] = orig_child;
                base_node->children[i]->pn = orig_child->pn;
                base_node->children[i]->dn = orig_child->dn;
                //display(child->get_board(), true);
            }
            else{
                std::cout<<"Child not found ("<<i<<")\n";
                //node->children[i]=nullptr;
                //new_tree.delete_all(child);
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
    Node* act_node = new PNSNode(board, tree.args);
    int act = -1;
    while(!tree.game_ended(act_node->get_board())){
        std::vector<int> color;
        //if((1ULL << act) & tree.heuristic.forbidden_all) build_licit_node(board, act);
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
                act_node = act_node->children[act];
            }
        }
        color.push_back(act);

        Board board(act_node->get_board());
        player = get_player(board.node_type);
        if(talky){
            printf("Action: %d pn: %d\n", act, (int)tree.get_states(board)->pn);
            display(board, true, color);
        }
        //std::cout<<"[DIFF] "<<board.white<<" "<<board.black<<std::endl;
        //print_board(board.white);
        //print_board(board.black);
    }
    std::cout<<"[END]\n";
}
