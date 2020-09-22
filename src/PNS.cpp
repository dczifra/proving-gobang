#include <fstream>

#include "PNS.h"
#include "limits.h"
#include "assert.h"
#include "common.h"
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
    //if(b.white_win(heuristic.all_linesinfo)){
    if((b.node_type == AND) && action > -1 && b.white_win(heuristic.linesinfo_per_field[action])){
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

void PNS::evaluate_components(PNSNode* node){
    assert(node->type == OR);

    display(node->board, true);

    Board::Artic_point p(&(node->board), heuristic.all_linesinfo, heuristic.linesinfo_per_field);
    auto comps = p.get_parts();
    int artic_point = std::get<0>(comps);
    std::cout<<artic_point<<std::endl;

    if(artic_point > -1){
        board_int comp1 = std::get<1>(comps);
        board_int comp2 = std::get<2>(comps);

        display(node->board, true, {artic_point});
        display(comp1, true, {artic_point});
        display(comp2, true, {artic_point});

        Board b_small(node->board);
        b_small.black |= comp2;

        // 1. Evalute smaller component without artic point
        PNSNode* small_comp = new PNS::PNSNode(b_small, node->depth+1, -1, -1, heuristic);
        evalueate_node_with_PNS(small_comp);
        add_state(small_comp);
        if(small_comp->pn == 0){
            delete_all(node);
            node = small_comp;
            return;
        }
        else{
            delete_all(small_comp);

            // 2. Evaluate small component with artic point
            (b_small.white) |= ((1ULL)<<artic_point);
            PNSNode* small_comp_mod = new PNS::PNSNode(b_small, node->depth, -1, -1, heuristic);
            add_state(small_comp_mod);
            evalueate_node_with_PNS(small_comp_mod);

            bool attacker_win = (small_comp_mod->pn == 0);
            delete_all(small_comp_mod);

            Board new_board(node->board);
            new_board.black |= comp1;
            // 3. If Attacker wins 
            if(attacker_win){
                new_board.white |= ((1ULL)<<artic_point);
            }

            int depth = node->depth;
            delete_all(node);
            //delete node;
            node = new PNS::PNSNode(new_board, depth, -1, -1, heuristic);
            add_state(node);

            //TODO: write to extend:
            // if(depth>...) evalueate_node_with_PNS(node);
        }
    }
}

void PNS::evalueate_node_with_PNS(PNSNode* node, bool log){
    int i=0;
    while(node->pn*node->dn != 0){
        PN_search(node);
        if(log && i%10000 == 0){
            stats(node);
        }
        ++i;
    }
}

void PNS::extend(PNS::PNSNode* node, unsigned int action){
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
    }
    // === Find next_state in reversed discovered states ===
    else if (states.find(reversed) != states.end()){
        node->children[action] = states[reversed];
        node->children[action] -> parent_num += 1;
    }
    else{
        int heur_val = 1;//(int) floor(pow(2.0, 8*(next_state.heuristic_val(heuristic.all_linesinfo)-1)));
        node->children[action] = new PNS::PNSNode(next_state, node->depth+1, last_act, heur_val, heuristic);
        states[next_state] = node->children[action];

        if(node->children[action]->type == OR && (node->children[action]->pn)*(node->children[action]->dn) != 0){
            //std::cout<<action<<" "<<last_act<<" "<<node->children[action]->pn<<" "<<node->children[action]->dn<<std::endl;
            //std::cout<<(next_state.node_type == AND)<<next_state.white_win(heuristic.linesinfo_per_field[action])<<std::endl;
            evaluate_components(node->children[action]);
        }
    }
}

void PNS::init_PN_search(PNS::PNSNode* node){
    add_state(node->board, node);
}

void PNS::PN_search(PNS::PNSNode* node){
    //assert(!node->board.white_win(heuristic.all_linesinfo));
    assert(node != nullptr);

    if(node->pn == 0 || node->dn == 0) return;

    if(node->type == OR){ // === OR  node ===
        unsigned int min_ind = get_min_children(node, PN, true);

        if(min_ind == (-1)) 0; // Disproof found
        else if(node->children[min_ind] == nullptr) extend(node, min_ind);
        else PN_search(node->children[min_ind]);
        // === Update PN and DN in node ===
        node->pn = get_min_children(node, PN);
        node->dn = get_sum_children(node, DN);
    }
    else{                 // === AND node ===
        unsigned int min_ind = get_min_children(node, DN, true);

        if(min_ind == (-1)) 0; // Proof found
        else if(node->children[min_ind] == nullptr) extend(node, min_ind);
        else PN_search(node->children[min_ind]);
        // === Update PN and DN in node ===
        node->pn = get_sum_children(node, PN);
        node->dn = get_min_children(node, DN);
    }

    // If PN or DN is 0, delete all unused descendents
    if(node->pn == 0 || node->dn == 0){
        delete_node(node);
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
void PNS::log_solution(std::string filename){
    std::ofstream log_file;
    log_file.open(filename);
    if (!log_file.is_open()) {
        std::cout<<"Problem with file (Check file/folder existency)\n";
    }

    for(auto & it: states){
        Board board = it.first;
        unsigned int pn = it.second->pn;
        unsigned int dn = it.second->dn;
        if(pn == 0 || dn == 0)
            log_file<<board.white<<" "<<board.black<<" "<<pn<<" "<<dn<<std::endl;
    }

    log_file.close();
}

void PNS::log_solution_min(PNS::PNSNode* node, std::ofstream& file){
    if(node == nullptr) return;
    else{
        file<<node->board.white<<" "<<node->board.black<<" "<<node->board.node_type<<" "<<node->pn<<" "<<node->dn<<std::endl;
        
        if( ((node->type == OR) && (node->pn == 0)) ||
            ((node->type == AND) && (node->dn==0)) ){
            ProofType proof_type = (node->pn == 0 ? PN:DN);
            unsigned int min_ind = get_min_children(node, proof_type, true);
            //assert(node->children[min_ind] != nullptr);
            if (min_ind == UINT_MAX) return;
            else log_solution_min(node->children[min_ind], file);
        }
        else{
            for(int i=0;i<ACTION_SIZE;i++){
                if(!node->board.is_valid(i)) continue;
                //assert(node->children[i] != nullptr);

                log_solution_min(node->children[i], file);
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
