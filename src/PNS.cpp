#include <fstream>

#include "PNS.h"
#include "limits.h"
#include "assert.h"
#include "common.h"
#include "artic_point.h"

#include <unistd.h>


PNS::PNSNode::PNSNode(const Board& b, unsigned int d, int action, int heur_val, Heuristic& h):children(), board(b), depth(d){
    type = b.node_type;
    unsigned int sum = 0;
    for(int i=0;i<ACTION_SIZE;i++){
        if(b.is_valid(i)) ++sum;
    }

    if(b.white_win(h.all_linesinfo)){
    //if((b.node_type == AND) && action > -1 && b.white_win(h.linesinfo_per_field[action])){
        pn = 0;
        dn = UINT_MAX;
    }
    else if((b.node_type == OR) && b.heuristic_stop(h.all_linesinfo)){
        pn = UINT_MAX;
        dn = 0;
    }
    // === Init proof and disploof number ===
    else if(sum == 0){ // Game is over, black wins
        pn = UINT_MAX;
        dn = 0;
    }
    else{
        pn = (type==OR ? 1:sum);
        //dn = (type==AND ? heur_val:sum);
        dn = (type==AND ? 1:sum);

        pn_th = pn;
        dn_th = dn;
    }

    parent_num = 1;
}

inline unsigned int get_child_value(PNS::PNSNode* child_node, const ProofType type){
    if(child_node == nullptr){
        return 1; // Improvement: Initialization policy
    }
    else if(type == PN){
        return child_node->pn;
    }
    else{
        return child_node->dn;
    }
}

unsigned int PNS::get_min_children(PNS::PNSNode* node, const ProofType type, bool index = false) const{
    unsigned int min = UINT_MAX;
    unsigned int min_ind = -1;

    for(int i=0;i<ACTION_SIZE;i++){
        if(!node->board.is_valid(i)) continue;

        unsigned int child = get_child_value(node->children[i], type);

        if(child < min ){
            min = child;
            min_ind = i;
        }
    }
    
    if(index) return min_ind;
    else return min;
}

unsigned int PNS::get_min_delta_index(PNS::PNSNode* node, int& second_ind) const{
    unsigned int first_min = UINT_MAX;
    unsigned int second_min = UINT_MAX;

    unsigned int first_ind = -1;
    second_ind = -1;

    for(int i=0;i<ACTION_SIZE;i++){
        if(!node->board.is_valid(i)) continue;
        // === take care of initialization ===
        unsigned int child = node->children[i] == nullptr ? 1ULL: node->children[i]->delta();

        if(child < second_min ){
            if(child < first_min){
                second_min = first_min;
                second_ind = first_ind;
                
                first_min = child;
                first_ind = i;
            }
            else{
                second_min = child;
                second_ind = i;
            }
        }
    }
    return first_ind;
}


unsigned int PNS::get_sum_children(PNS::PNSNode* node, const ProofType type) const{
    unsigned int sum = 0;

    for(int i=0;i<ACTION_SIZE;i++){
        if(!node->board.is_valid(i)) continue;
        
        unsigned int child = get_child_value(node->children[i], type);

        if(child == UINT_MAX) return UINT_MAX;
        sum += child;
    }
    return sum;
}

bool PNS::game_ended(const Board& b, int action){
    if(b.white_win(heuristic.all_linesinfo)){
    //if((b.node_type == AND) && action > -1 && b.white_win(heuristic.linesinfo_per_field[action])){
        return true;
    }
    else if((b.node_type == OR) && b.heuristic_stop(heuristic.all_linesinfo)){
        return true;
    }
    return false;
}

inline void PNS::simplify_board(Board& next_state, const unsigned int action, int depth){
    if(next_state.node_type == OR){ // Last move: black

        next_state.remove_lines_with_two_ondegree(heuristic.all_linesinfo);
        next_state.remove_2lines_all(heuristic.all_linesinfo);
        next_state.remove_dead_fields_all(heuristic.all_linesinfo);

        //next_state.get_one_artic_point(heuristic);
    }
    else{
        //next_state.keep_comp(heuristic.linesinfo_per_field, action);
    }
}

PNS::PNSNode* PNS::create_and_eval_node(Board& board, int base_depth, bool eval, bool search = false){
    PNSNode* node;
    Board flipped(board);
    flipped.flip();

    if(states.find(board) != states.end()){
        node = states[board];
        node -> parent_num += 1;
    }
    else if(states.find(flipped) != states.end()){
        node = states[flipped];
        node -> parent_num += 1; 
    }
    else{
        node = new PNS::PNSNode(board, base_depth, -1, -1, heuristic);
        states[board] = node;
        //add_state(node);
    }

    if(eval) evalueate_node_with_PNS(node, false, false);
    return node;
}

PNS::PNSNode* PNS::evaluate_components(Board& base_board, const int base_depth){
    assert(base_board.node_type == OR);
    bool delete_comps = true;
    //display(base_board, true);

    Artic_point comps(base_board, heuristic.all_linesinfo, heuristic.linesinfo_per_field);
    int artic_point;
    Board small_board, big_board;
    std::tie(artic_point, small_board, big_board) = comps.get_parts();

    if(artic_point > -1){
        //display(base_board, true, {artic_point});
        //display(comp1, true, {artic_point});
        //display(comp2, true, {artic_point});
        //#if TALKY
        //    std::cout<<"Artic point: "<<artic_point<<std::endl;
        //    display(base_board, true);
        //#endif

        // 1. Evalute smaller component without artic point
        PNSNode* small_comp = create_and_eval_node(small_board, base_depth, true);

        if(small_comp->pn == 0){
            #if TALKY
                std::cout<<"Small comp\n";
                display(small_board, true);
            #endif
            //std::cout<<" "<<__builtin_popcountll(small_board.get_valids())<<std::endl;
            return small_comp;
        }
        else{
            if (delete_comps) delete_all(small_comp);

            // 2. Evaluate small component with artic point
            (small_board.white) |= ((1ULL)<<artic_point);
            PNSNode* small_comp_mod = create_and_eval_node(small_board, base_depth, true);

            // 3-4. If Attacker wins: C* else C
            //display(small_board.black, true);
            //display(small_board.white, true);
            //std::cout<<small_comp_mod->pn<<std::endl;

            if(small_comp_mod->pn == 0){
                big_board.white |= ((1ULL)<<artic_point);
            }
            if (delete_comps) delete_all(small_comp_mod);

            #if TALKY
                std::cout<<"Big comp\n";
                display(big_board, true);
            #endif

            // TODO: Improve...
            bool eval_big = __builtin_popcountll(big_board.get_valids()) < EVAL_TRESHOLD;
            //if(eval_big) display(big_board, true);
            PNSNode* big_comp = create_and_eval_node(big_board, base_depth, eval_big);
            return big_comp;
        }
    }
    else{
        PNSNode* node = new PNSNode(base_board, base_depth, -1, -1, heuristic);
        states[base_board] = node;
        return node;
    }
}

void PNS::evalueate_node_with_PNS(PNSNode* node, bool log, bool fast_eval){
    int i=0;
    while(node->pn*node->dn != 0){
        PN_search(node, fast_eval);
        if(log && i%10000 == 0){
            stats(node);
        }
        ++i;
    }
}

Board PNS::extend(PNS::PNSNode* node, unsigned int action, bool fast_eval){
    Board next_state(node->board, action, get_player(node->type));

    simplify_board(next_state, action, node->depth);
    //simplify_board(next_state, action, node->depth);

    int last_act = action;
    while(!game_ended(next_state, last_act)){
        int temp_act = next_state.one_way(get_all_lines());
        if(temp_act > -1){
            last_act = temp_act;
            next_state.move(last_act, next_state.node_type== OR ? 1 : -1);
        }
        else break;
    }
    
    Board reversed(next_state);
    reversed.flip();

    // === Find next_state in discovered states ===
    if(states.find(next_state) != states.end()){
        node->children[action] = states[next_state];
        node->children[action] -> parent_num += 1;
        return next_state;
    }
    // === Find next_state in reversed discovered states ===
    else if (states.find(reversed) != states.end()){
        node->children[action] = states[reversed];
        node->children[action] -> parent_num += 1;
        return next_state;
    }
    else{
        assert(node->children[action] == nullptr);
        int heur_val = 1;//(int) floor(pow(2.0, 8*(next_state.heuristic_val(heuristic.all_linesinfo)-1)));

        // 2-connected componets, if not ended
        if(!fast_eval && next_state.node_type == OR && !game_ended(next_state, last_act)){
            //std::cout<<"Next state\n";
            //display(next_state, true);
            node->children[action] = evaluate_components(next_state, node->depth+1);
        }
        else{
            node->children[action] = new PNS::PNSNode(next_state, node->depth+1, last_act, heur_val, heuristic);
            states[next_state] = node->children[action];
            if(fast_eval && __builtin_popcountll(next_state.get_valids()) < EVAL_TRESHOLD){
                evalueate_node_with_PNS(node->children[action], false, false);
            }
        }
        return node->children[action]->board;
    }
}

void PNS::init_PN_search(PNS::PNSNode* node){
    add_state(node->board, node);
}

void PNS::PN_search(PNS::PNSNode* node, bool fast_eval){
    //assert(!node->board.white_win(heuristic.all_linesinfo));
    assert(node != nullptr);

    if(node->pn == 0 || node->dn == 0) return;

    if(node->type == OR){ // === OR  node ===
        unsigned int min_ind = get_min_children(node, PN, true);

        if(min_ind == (-1)) 0; // Disproof found
        else if(node->children[min_ind] == nullptr) extend(node, min_ind, fast_eval);
        else PN_search(node->children[min_ind], fast_eval);
        // === Update PN and DN in node ===
        node->pn = get_min_children(node, PN);
        node->dn = get_sum_children(node, DN);
    }
    else{                 // === AND node ===
        unsigned int min_ind = get_min_children(node, DN, true);

        if(min_ind == (-1)) 0; // Proof found
        else if(node->children[min_ind] == nullptr) extend(node, min_ind, fast_eval);
        else PN_search(node->children[min_ind], fast_eval);
        // === Update PN and DN in node ===
        node->pn = get_sum_children(node, PN);
        node->dn = get_min_children(node, DN);
    }

    // If PN or DN is 0, delete all unused descendents
    if(node->pn == 0 || node->dn == 0){
        delete_node(node);
    }
}

void PNS::delete_children(PNS::PNSNode* node){
    if( node->parent_num>1 ){
        node->parent_num -= 1;
    }
    else{
        for(int i=0;i<ACTION_SIZE;i++){
            if((!node->board.is_valid(i)) || (node->children[i]==nullptr)) continue;
            else{
                delete_all(node->children[i]);
                node->children[i]=nullptr;
            }
        }
    }
}

void PNS::delete_all(PNS::PNSNode* node){
    if( node->parent_num>1 ){
        node->parent_num -= 1;
    }
    else{
        for(int i=0;i<ACTION_SIZE;i++){
            if((!node->board.is_valid(i)) || (node->children[i]==nullptr)) continue;
            else{
                delete_all(node->children[i]);
                node->children[i]=nullptr;
            }
        }
        states.erase(node->board);
        delete node;
    }
}

void PNS::delete_node(PNS::PNSNode* node){
    // === In this case, we need max 1 branch ===
    if( ((node->type == OR) && (node->pn == 0)) ||
        ((node->type == AND) && (node->dn==0)) ){
        
        ProofType proof_type = (node->pn == 0 ? PN:DN);
        unsigned int min_ind = get_min_children(node, proof_type, true);

        assert(min_ind !=-1);
        // === Delete all children, except min_ind
        for(int i=0;i<ACTION_SIZE;i++){
            if((!node->board.is_valid(i)) || (node->children[i]==nullptr) ||
                (min_ind == i)) continue;
            else{
                delete_all(node->children[i]);
                node->children[i]=nullptr;
            }
        }
    }
    // ELSE: keep all children, because we need all of them
}

// === Helper Functions ===
void PNS::log_solution_min(PNS::PNSNode* node, std::ofstream& file, std::set<Board>& logged){
    if(node == nullptr) return;
    else if(logged.find(node->board) == logged.end()){
        logged.insert(node->board);
        file<<node->board.white<<" "<<node->board.black<<" "<<node->board.node_type<<" "<<node->pn<<" "<<node->dn<<std::endl;
        
        if( ((node->type == OR) && (node->pn == 0)) ||
            ((node->type == AND) && (node->dn==0)) ){
            ProofType proof_type = (node->pn == 0 ? PN:DN);
            unsigned int min_ind = get_min_children(node, proof_type, true);
            if (min_ind == UINT_MAX) return;
            else log_solution_min(node->children[min_ind], file, logged);
        }
        else{
            for(int i=0;i<ACTION_SIZE;i++){
                if(!node->board.is_valid(i)) continue;

                log_solution_min(node->children[i], file, logged);
            }
        }
    }
}


void PNS::add_state(const Board& b, PNS::PNSNode* node){
    if(states.find(b)==states.end()){
        states[b]=node;
    }
}

void PNS::add_state(PNS::PNSNode* node){
    if(states.find(node->board)==states.end()){
        states[node->board]=node;
    }
}

void PNS::free_states(){
    for(auto node_it: states){
        delete node_it.second;
    }
}
