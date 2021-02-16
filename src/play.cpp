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
    read_solution(filename, tree);
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

void side_starts(Board& board){
    // === LEFT ===
    //std::vector<int> whites = {1,3,8};
    //std::vector<int> blacks = {7};
    //std::vector<int> whites = {1,3};
    //std::vector<int> blacks = {2};

    // === RIGHT ===
    //std::vector<int> whites2 = {43,46,48};
    //std::vector<int> blacks2 = {42};
    //std::vector<int> whites2 = {46, 48};
    //std::vector<int> blacks2 = {};
    // === Middle start ====
    //a
    //std::vector<int> whites2 = {42, 46};
    //std::vector<int> blacks2 = {41, 48};
    //std::vector<int> whites2 = {41, 48};
    //std::vector<int> blacks2 = {43, 46};
    //b
    //std::vector<int> whites2 = {42, 48};
    //std::vector<int> blacks2 = {41, 46};
    //std::vector<int> whites2 = {48};
    //std::vector<int> blacks2 = {46};

    //std::vector<int> whites = {1, 7,3};
    //std::vector<int> blacks = {2, 11};
    //std::vector<int> whites2 = {41, 47};
    //std::vector<int> blacks2 = {42};
    
    std::vector<int> whites = {46,48};
    std::vector<int> blacks = {47};
    //std::vector<int> whites2 = {47, 42};
    //std::vector<int> blacks2 = {37};
    //std::vector<int> whites2 = {42, 43, 46};
    //std::vector<int> blacks2 = {41, 37};
    
    std::vector<int> whites2 = {7,11};
    std::vector<int> blacks2 = {12,8};

    //std::vector<int> whites2 = {2,6,8};
    //std::vector<int> blacks2 = {7};
    board.forbidden_all = 0;

    for(auto w: whites) board.white |= (1ULL << w);
    for(auto b: blacks) board.black |= (1ULL << b);
    for(auto w: whites2) board.white |= (1ULL << w);
    for(auto b: blacks2) board.black |= (1ULL << b);
}

NodeType Play::choose_problem(Board& board, int& player, bool disproof, Args* args){
    if(disproof) board.move({0,1, ACTION_SIZE-1}, player);
    //board.move({7,11}, player);

    //side_starts(board);
    board.white |= (1ULL << 0) | (1ULL << 45);
    //board.white |= (1ULL << 45);
    //board.black |= (1ULL << 0);

    if(args->START > -1) board.move({args->START}, player);

    return (player==1?OR:AND);
}

void Play::read_solution(std::string filename, PNS& mytree){
    Args temp_args;
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
        sstream>>b.white>>b.black>>b.node_type>>b.score_left>>b.score_right>>b.forbidden_all>>pn>>dn;
        if(mytree.get_states(b)==nullptr){
            
            PNSNode* node = new PNSNode(b, &temp_args);
            node->pn = pn;
            node->dn = dn;
            mytree.add_board(b, node);
            //display(b, true);
            //std::cout<<node->get_board().white<<" "<<node->get_board().black<<" "<<node->get_board().node_type<<" "<<node->get_board().forbidden_all<<"\n";
   
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

// void Play::build_node(Board b){
//     PNS new_tree(args);
//     int slot = 0;
    
//     board_int valids = b.get_valids_without_ondegree(PNS::heuristic.all_linesinfo);
//     for(int i=0;i<ACTION_SIZE;i++){
//         if(valids & (1ULL << i)){
//             Board next = new_tree.extend(tree.get_states(b), i, slot, false);
//             PNSNode* child = tree.get_states(next);
//             if(child != nullptr){
//                 tree.get_states(b)->children[slot] = child;
//             }
//             slot++;
//         }
//     }
// }

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

void Play::play_with_solution(){
    Board base_board;
    choose_problem(base_board, player, false, args); // TODO
    Node* act_node = new PNSNode(base_board, tree.args);
    int act = -1;
    while(act_node->is_inner() || !tree.game_ended(act_node->get_board())){
        //std::cout<<"Childnum: "<<act_node->child_num<<std::endl;
        //tree.evaluate_node_with_PNS(act_node, true, false);
        //printf("PN: %f, DN: %f\n", act_node->pn, act_node->dn);
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
            }
            else
            {
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
                //std::cout<<"Scores: "<<board.score_left<<" "<<board.score_right<<std::endl;
            }
            //std::cout<<"[DIFF] "<<board.white<<" "<<board.black<<std::endl;
            //print_board(board.white);
            //print_board(board.black);
        }
        //std::cout<<"next done\n";
    }
    std::cout<<"[END]\n";
}
