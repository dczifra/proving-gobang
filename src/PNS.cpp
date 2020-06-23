#include <fstream>

#include "PNS.h"
#include "limits.h"
#include "assert.h"
#include "common.h"

NodeType operator!(const NodeType& type){
    return (type==OR?AND:OR);
}

int get_player(const NodeType& type){
    return (type==OR?1:-1);
}

PNSNode::PNSNode(const Board& b, NodeType t):children(), board(b), type(t){
    unsigned int sum = 0;
    for(int i=0;i<ACTION_SIZE;i++){
        if(b.is_valid(i)) ++sum;
    }

    // === Init proof and disploof number ===
    if(sum == 0){ // Game is over, black wins
        pn = UINT_MAX;
        dn = 0;
    }
    else{
        pn = (t==OR ? 1:sum);
        dn = (t==AND ? 1:sum);
    }

    parent_num = 1;
}

inline unsigned int get_child_value(PNSNode* child_node, const ProofType type){
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

unsigned int PNS::get_min_children(PNSNode* node, const ProofType type, bool index = false) const{
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

unsigned int PNS::get_sum_children(PNSNode* node, const ProofType type) const{
    unsigned int sum = 0;

    for(int i=0;i<ACTION_SIZE;i++){
        if(!node->board.is_valid(i)) continue;
        
        unsigned int child = get_child_value(node->children[i], type);

        if(child == UINT_MAX) return UINT_MAX;
        sum += child;
    }
    return sum;
}

void PNS::extend(PNSNode* node, const unsigned int action){
    Board next_state(node->board, action, get_player(node->type));
    if((node->type == AND)){
        next_state.remove_small_components(heuristic.fields_on_compressed_lines);
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
        NodeType t = !(node->type);
        node->children[action] = new PNSNode(next_state, t);

        if((node->type == OR) && next_state.white_win(get_lines(action))){
            node->children[action]->pn = 0;
            node->children[action]->dn = UINT_MAX;
        }
        //else if((node->type == AND) && next_state.black_win(get_all_lines())){
        //else if((node->type == AND) && next_state.no_free_lines(get_all_lines())){
        else if((node->type == AND) && next_state.heuristic_stop(get_all_lines())){
            node->children[action]->pn = UINT_MAX;
            node->children[action]->dn = 0;
        }
        states[next_state] = node->children[action];
    }

}

void PNS::search(PNSNode* node){
    assert(node != nullptr);
    if(node->pn == 0 || node->dn == 0) return;

    if(node->type == OR){ // === OR  node ===
        unsigned int min_ind = get_min_children(node, PN, true);

        if(min_ind == (-1)) 0; // Disproof found
        else if(node->children[min_ind] == nullptr) extend(node, min_ind);
        else search(node->children[min_ind]);
        // === Update PN and DN in node ===
        node->pn = get_min_children(node, PN);
        node->dn = get_sum_children(node, DN);

    }
    else{                 // === AND node ===
        unsigned int min_ind = get_min_children(node, DN, true);

        if(min_ind == (-1)) 0; // Proof found
        else if(node->children[min_ind] == nullptr) extend(node, min_ind);
        else search(node->children[min_ind]);
        // === Update PN and DN in node ===
        node->pn = get_sum_children(node, PN);
        node->dn = get_min_children(node, DN);
    }

    // If PN or DN is 0, delete all unused descendants
    if(node->pn == 0 || node->dn == 0){
        //delete_node(node);
    }
}

void PNS::delete_all(PNSNode* node){
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
void PNS::delete_node(PNSNode* node){
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

void PNS::log_solution_min(PNSNode* node, std::ofstream& file){
    if(node == nullptr) return;
    else{
        file<<node->board.white<<" "<<node->board.black<<" "<<node->pn<<" "<<node->dn<<std::endl;
        
        if( ((node->type == OR) && (node->pn == 0)) ||
            ((node->type == AND) && (node->dn==0))){
            ProofType proof_type = (node->pn == 0 ? PN:DN);
            unsigned int min_ind = get_min_children(node, proof_type, true);
            //assert(node->children[min_ind] != nullptr);
            log_solution_min(node->children[min_ind], file);
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

void PNS::add_state(Board& b, PNSNode* node){
    if(states.find(b)==states.end()){
        states[b]=node;
    }
}

void PNS::free_states(){
    for(auto node_it: states){
        delete node_it.second;
    }
}