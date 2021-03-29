#include "play.h"
#include "artic_point.h"
#include "logger.h"

#include<sstream>
#include<fstream>

Play::Play(std::string filename, bool disproof, bool talky, Args* args_):talky(talky),tree(args_){
    player = 1;
    args = args_;

    // === Read Solution Tree ===
    //Board board;
    //choose_problem(board, player, disproof, args);

    read_solution(filename, tree);
    printf("Proof/disproof tree size: %zu\n", tree.states.size());
    printf("Isommap size: %zu\n", isom_map.size());
    human_player = -1;
}

void side_starts(Board& board){
    std::vector<int> whites = {46,48};
    std::vector<int> blacks = {47};

    std::vector<int> whites2 = {2,8,6};
    std::vector<int> blacks2 = {7};
    
    board.forbidden_all = 0;
    //board.node_type = AND;

    for(auto w: whites) board.white |= (1ULL << w);
    for(auto b: blacks) board.black |= (1ULL << b);
    for(auto w: whites2) board.white |= (1ULL << w);
    for(auto b: blacks2) board.black |= (1ULL << b);
}

NodeType Play::choose_problem(Board& board, int& player, bool disproof, Args* args){
    if(disproof) board.move({0,1, ACTION_SIZE-1}, player);
    
    board.forbidden_all = 0;

    if(args->START > -1) board.move({args->START}, player);

    return (player==1?OR:AND);
}

void Play::read_solution(std::string filename, PNS& mytree){
    Args temp_args;
    std::ifstream file(filename.c_str(), std::ifstream::in | std::ifstream::binary);
    if(file.is_open()){
        std::cout<<"\rProcessing file...: "<<filename<<"        "<<std::flush;
    }
    else{
        std::cout<<"File not found: "<<filename<<std::endl;
    }

    std::string s;
    while(file){
        //std::cout<<"www"<<std::endl;
        int pn,dn;
        Board b;
        file.read((char*) &b, sizeof(Board));
        file.read((char*) &pn, sizeof(int));
        file.read((char*) &dn, sizeof(int));
        if(mytree.get_states(b)==nullptr){
            
            PNSNode* node = new PNSNode(b, &temp_args);
            node->pn = pn;
            node->dn = dn;
            mytree.add_board(b, node);
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

void Play::build_node2(PNSNode* base_node){
    PNS new_tree(args);
    PNSNode* node = new PNSNode(base_node->board, args);
    new_tree.extend_all(node, false);
    for(unsigned int i=0;i<node->children.size();i++){
        Node* child = node->children[i];
        if(child == nullptr) return; // Extends stops, if node is proven
        else if(child->is_inner()){
            //std::cout<<"Child extended ("<<i<<" "<<child->pn<<")\n";
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

void info_to_plot(const Board& board){
    std::cout<<"[DIFF] "<<board.white<<" "<<board.black<<std::endl;
    print_board(board.white);
    print_board(board.black);
}

void Play::play_with_solution(){
    Board base_board;
    choose_problem(base_board, player, false, args); // TODO
    Node* act_node = new PNSNode(base_board, tree.args);
    //tree.extend_all((PNSNode*)act_node, false);
    //act_node = act_node->children[0];
    if(talky) display(base_board, true);
    else info_to_plot(base_board);
    std::cout<<base_board.heuristic_value(PNS::heuristic.all_linesinfo)<<std::endl;
    
    int act = -1;
    while(act_node->is_inner() || !tree.game_ended(act_node->get_board())){
        //std::cout<<"Childnum: "<<act_node->child_num<<std::endl;
        if(!act_node->is_inner()){
            //std::cout<<act_node->get_board().white<<" "<<act_node->get_board().black<<std::endl;
            //display(act_node->get_board().forbidden_all, true);
        }

        std::vector<int> color;
        act = -1;
        if(!act_node->is_inner()) build_node2((PNSNode*)act_node);
        // === Human player can choose ===
        if(player == human_player ){
            int row, col;
            std::cout<<"[RES]\n";
            if(act_node->is_inner()){
                printf("Your choices\n");
                std::vector<Board> boards;
                for(auto ch: act_node->children) boards.push_back(ch->get_board());

                display(boards, true);
                std::cin>>act;
                act_node = act_node->children[act];
            }
            else{
                std::cin>>row>>col;
                act = col*ROW+row;
                act_node = act_node->children[get_index(act, act_node->get_board())];
            }
        }
        else{
            // === Find the next child in Solution Tree ===
            if(act_node->type == OR) act = tree.get_min_children_index(act_node, PN);
            else act = tree.get_min_children_index(act_node, DN);

            if(act == ACTION_SIZE or act == -1){
                printf("Not found next step %d\n", act);
                std::string filename = "data/board_sol/"+act_node->get_board().to_string()+".sol";
                tree.stats(act_node, true);
                Play::read_solution(filename, tree);
                tree.stats(act_node, true);
                //std::cout<<"[END]\n";
                //return;
                //tree.evaluate_node_with_PNS(act_node, true, false);
                //printf("PN: %f, DN: %f\n", act_node->pn, act_node->dn);
                //if(act_node->type == OR) act = tree.get_min_children_index(act_node, PN);
                //else act = tree.get_min_children_index(act_node, DN);
            }
            else{
                int true_act = act_node->is_inner() ? act : get_action(act, act_node->get_board());
                act_node = act_node->children[act];
                act = true_act;
            }
        }
        color.push_back(act);

        if(act_node->is_inner()){
            //std::cout<<"Inner node\n";
            player = get_player(act_node->type);
        }
        else{
            Board board(act_node->get_board());
            player = get_player(board.node_type);
            if(talky){
                printf("Action: %d pn: %d\n", act, (int)tree.get_states(board)->pn);
                display(board, true, color);
                std::cout<<board.heuristic_value(PNS::heuristic.all_linesinfo)<<std::endl;
                //std::cout<<"Scores: "<<board.score_left<<" "<<board.score_right<<std::endl;
            }
            else info_to_plot(board);
        }
    }
    std::cout<<"[END]\n";
}
