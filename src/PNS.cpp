#include <fstream>

#include "PNS.h"
#include "limits.h"
#include "assert.h"
#include "common.h"
#include "artic_point.h"
#include "logger.h"

#include <unistd.h>

PNS::PNSNode::PNSNode(const Board& b, Heuristic& h):children(), board(b){
    type = b.node_type;
    child_num = 0;
    for(int i=0;i<ACTION_SIZE;i++){
        if(b.is_valid(i)) ++child_num;
    }

    children.resize(child_num);
    heuristic_value = board.heuristic_value(h.all_linesinfo);
    
    if(b.white_win(h.all_linesinfo)){
        pn = 0;
        dn = var_MAX;
    }
    else if((b.node_type == AND) && b.heuristic_stop(h.all_linesinfo)){
        pn = var_MAX;
        dn = 0;
    }
    // === Init proof and disploof number ===
    else if(child_num == 0){ // Game is over, black wins
        pn = var_MAX;
        dn = 0;
    }
    else{
        init_pn_dn();
    }

    parent_num = 1;
}

void inline PNS::PNSNode::init_pn_dn(){
#if HEURISTIC
    // pn = 1/(1+exp(3*heuristic_value-2));
    // int valid_moves = __builtin_popcountll(board.get_valids());
    // pn = std::pow(100, valid_moves / heuristic_value);
    // pn = std::pow(1, valid_moves - 15* heuristic_value);
    pn = (type==OR ? 1:child_num);

    // dn = 1/(1+exp(-3*heuristic_value-2));
    dn = std::pow(1000, heuristic_value);
    // dn = (type==AND ? 1:child_num);
#else
    pn = (type==OR ? 1:child_num);
    dn = (type==AND ? 1:child_num);
#endif
}

inline var get_child_value(PNS::PNSNode* child_node, const ProofType type){
  assert(child_node != nullptr);

  if(type == PN){
    return child_node->pn;
  }
  else{
    return child_node->dn;
  }
}

unsigned int PNS::get_min_children_index(PNS::PNSNode* node, const ProofType type){
  var min = var_MAX;
  unsigned int min_ind = -1;
  
  for(int i=0;i<node->child_num;i++){
    if (node->children[i] == nullptr) continue;
    var child = get_child_value(node->children[i], type);
    // std::cout<<"Child: "<<i<<" value: "<<child<< " dn: "<<node->children[i]->dn<< " extended: "<<node->children[i]->extended<<" heur val: "<<node->children[i]->heuristic_value<<std::endl;

    if(child < min ){
      min = child;
      min_ind = i;
    }
  }
  
  return min_ind;
}

var PNS::get_min_children(PNS::PNSNode* node, const ProofType type) const{
  var min = var_MAX;
  
  for(int i=0;i<node->child_num;i++){
    if (node->children[i] == nullptr) continue;
    var child = get_child_value(node->children[i], type);
    
    if(child < min ){
      min = child;
    }
  }
  return min;
}

var PNS::get_sum_children(PNS::PNSNode* node, const ProofType type) const{
  var sum = 0;

  for(int i=0;i<node->child_num;i++){
    if (node->children[i] == nullptr) continue;
    
    var child = get_child_value(node->children[i], type);
    if(child == var_MAX) return var_MAX;
    sum += child;
  }
  return sum;
}

bool PNS::keep_only_one_child(PNS::PNSNode* node){
    return ((node->type == OR) && (node->pn == 0)) ||
        ((node->type == AND) && (node->dn==0));
}

bool PNS::game_ended(const Board& b){
    if(b.white_win(heuristic.all_linesinfo)){
        return true;
    }
    else if((b.node_type == AND) && b.heuristic_stop(heuristic.all_linesinfo)){
        return true;
    }
    else if(b.white == 0 && b.black == FULL_BOARD){
        return true;
    }
    return false;
}

void PNS::simplify_board(Board& next_state){
    while(!game_ended(next_state)){
        if(next_state.node_type == OR){ // Last move: black
            next_state.remove_lines_with_two_ondegree(heuristic.all_linesinfo);
            next_state.remove_2lines_all(heuristic.all_linesinfo);
            next_state.remove_dead_fields_all(heuristic.all_linesinfo);
        }
        int temp_act = next_state.one_way(get_all_lines());
        if(temp_act > -1){
            next_state.move(temp_act, next_state.node_type== OR ? 1 : -1);
        }
        else{
            break;
        }
    }
}

PNS::PNSNode* PNS::create_and_eval_node(Board& board, bool eval, bool search = false){
    PNSNode* node;

    node = get_states(board);
    if(node != nullptr){
        node -> parent_num += 1;
    }
    else{
        node = new PNS::PNSNode(board, heuristic);
        add_board(board, node);
    }

    if(eval){
      evaluate_node_with_PNS(node, false, false);
    }
    return node;
}

PNS::PNSNode* PNS::evaluate_components(Board& base_board){
    assert(base_board.node_type == OR);
    bool delete_comps = true;
    
    int artic_point;
    Board small_board, big_board;
    Artic_point comps(base_board, heuristic.all_linesinfo, heuristic.linesinfo_per_field);
    std::tie(artic_point, small_board, big_board) = comps.get_parts();

    if(artic_point > -1){
        // 1. Evalute smaller component without artic point
        Board small_board_with_artic(small_board);        
        simplify_board(small_board);
        PNSNode* small_comp = create_and_eval_node(small_board, true);

        if(small_comp->pn == 0){
            return small_comp;
        }
        else{
          if (delete_comps) {
            delete_all(small_comp);
          }

            // 2. Evaluate small component with artic point
            // (small_board.white) |= ((1ULL)<<artic_point);
            // PNSNode* small_comp_mod = create_and_eval_node(small_board, true);
            (small_board_with_artic.white) |= ((1ULL)<<artic_point);
            simplify_board(small_board_with_artic);
            PNSNode* small_comp_mod = create_and_eval_node(small_board_with_artic, true);

            // 3-4. If Attacker wins: C* else C
            if(small_comp_mod->pn == 0){
                big_board.white |= ((1ULL)<<artic_point);
            }
            if (delete_comps){
              delete_all(small_comp_mod);
            }

            // TODO: Improve...
            simplify_board(big_board);
            PNSNode* big_comp = create_and_eval_node(big_board, false);
            return big_comp;
        }
    }
    else{
        PNSNode* node = new PNSNode(base_board, heuristic);
        add_board(base_board, node);
        return node;
    }
}

void PNS::evaluate_node_with_PNS(PNSNode* node, bool log, bool fast_eval){
    int i=0;
    while(node->pn*node->dn != 0){
        PN_search(node, fast_eval);

        if(log && i%10000 == 0){
            stats(node);
        }
        ++i;
    }
}

void PNS::extend_all(PNS::PNSNode* node, bool fast_eval){
    // std::cout<<"extend_all"<<std::endl;
    if(node == nullptr) std::cout<<"died in extend_all"<<std::endl;
    if((node->pn == 0) || (node->dn == 0) || node->extended) return;
    int slot = 0;
    for(int i=0;i<ACTION_SIZE;i++){
        if(node->board.is_valid(i)){
            extend(node, i, slot, fast_eval);
            // if(node->children[slot]->type == OR){
            //   extend_all(node->children[slot], fast_eval);
            // }
            if ((node->type == OR) and (node->children[slot]->pn==0) or (node->type == AND) and (node->children[slot]->dn==0)) break;
            slot++;
        }
    }
    node->extended = true;

#if HEURISTIC
    // default DN is not useful in OR nodes, so we update them
    if(node->type == AND){
        float heur_parent = node->heuristic_value;
        float heur_min = FLT_MAX;
        for(int i=0;i<node->child_num;i++){
            if(node->children[i] == nullptr) continue;
            if(node->children[i]->type == OR){
                heur_min = std::min(heur_min, node->children[i]->heuristic_value);
            }
        }
        PNSNode* current_child;
        for(int i=0;i<node->child_num;i++){
            if(node->children[i] == nullptr) continue;
            current_child = node->children[i];
            if(current_child->type == OR and current_child->dn != 0 and current_child->pn != 0){
                current_child->dn = std::pow(1000, heur_parent - heur_min + current_child->heuristic_value);
            }
        }
    }
        
#endif

    update_node(node);
}

Board PNS::extend(PNS::PNSNode* node, unsigned int action, unsigned int slot, bool fast_eval){
    Board next_state(node->board, action, get_player(node->type));

    simplify_board(next_state);
    
    PNSNode* child = get_states(next_state);
    if(child != nullptr){
        node->children[slot] = child;
        node->children[slot] -> parent_num += 1;
        return next_state;
    }
    else{
        assert(node->children[slot] == nullptr);

        // 2-connected components, if not ended
        int moves_before = __builtin_popcountll(next_state.get_valids());
        if(!fast_eval && next_state.node_type == OR && !game_ended(next_state) && moves_before >= EVAL_TRESHOLD){ 
            node->children[slot] = evaluate_components(next_state);
        } 
        else{
            node->children[slot] = new PNS::PNSNode(next_state, heuristic);
            add_board(next_state, node->children[slot]);
            if(!fast_eval && moves_before < EVAL_TRESHOLD){
                evaluate_node_with_PNS(node->children[slot], false, true);
            }
        }
        int moves_after = __builtin_popcountll(node->children[slot]->board.get_valids());
        
        component_cut[moves_before][moves_before - moves_after] += 1;

        return node->children[slot]->board;
    }
}

void PNS::init_PN_search(PNS::PNSNode* node){
    add_board(node->board, node);
}

void PNS::log_node(PNS::PNSNode* node){
    if(keep_only_one_child(node) && node->child_num > 0){
        int ind = rand() % node->child_num;
        if(node->children[ind] != nullptr && __builtin_popcountll(node->children[ind]->board.get_valids()) < ROW*COL/2){
            evaluate_node_with_PNS(node->children[ind]);
            logger->log(node->children[ind], heuristic);
        }
    }
    logger->log(node, heuristic);
}

void PNS::PN_search(PNS::PNSNode* node, bool fast_eval){
    assert(node != nullptr);
    // display_node(node);
    if(node->pn == 0 || node->dn == 0) return;

    // if we are in a leaf, we extend it
    if(!node->extended){
      extend_all(node, fast_eval);
    }
    else{
      unsigned int min_ind = get_min_children_index(node, node->type == OR?PN:DN);

      // std::cout<<std::endl<<node->pn<<" - "<<node->dn<<std::endl;
      // std::cout<<node->child_num<<std::endl;
      // std::cout<<"Step: "<<min_ind<<std::endl;
      if (min_ind != -1){
        PN_search(node->children[min_ind], fast_eval);
      }
      update_node(node);
    }

    // If PN or DN is 0, delete all unused descendents
    if(node->pn == 0 || node->dn == 0){
      #if LOG
      log_node(node);
      #endif
      delete_node(node);
    }
}

void PNS::update_node(PNS::PNSNode* node){
  // std::cout<<"updating from:"<<node->pn<<" "<<node->dn<<std::endl;
        

  // === Update PN and DN in node ===
  if(node->type == OR){
    node->pn = get_min_children(node, PN);
    node->dn = get_sum_children(node, DN);
  }
  else{
    node->pn = get_sum_children(node, PN);
    node->dn = get_min_children(node, DN);
  }
}

// ============================================
//             DELETE UNUSED NODES
// ============================================
void PNS::delete_all(PNS::PNSNode* node){
  // std::cout<<"beleptem"<<std::endl;
  if(node == nullptr) return;
  // std::cout<<"extended "<<node->extended<<std::endl;
  // std::cout<<"parent_num "<<node->parent_num<<std::endl;
  if( node->parent_num > 1 ){
    node->parent_num -= 1;
  }
  else{
    // display(node->board, true);
    // std::cout<<"child_num "<<node->child_num<<std::endl;
    // std::cout<<"numbers:"<<node->pn<<" "<<node->dn<<std::endl;
    for(int i=0;i<node->child_num;i++){
      //          std::cout<<"child "<<i<<std::endl;
      delete_all(node->children[i]);
      node->children[i]=nullptr;
    }
    
    // std::cout<<"before delete_from_map"<<std::endl;
    delete_from_map(node->board);
    delete node;
  }
  //std::cout<<"kileptem"<<std::endl;
}

  //We only keep a single child that proves the given node
void PNS::delete_node(PNS::PNSNode* node){
    // === In this case, we need max 1 branch ===
    // === DONT DELTE THIS IF ===
    if(keep_only_one_child(node)){
        
        ProofType proof_type = (node->pn == 0 ? PN:DN);
        unsigned int min_ind = get_min_children_index(node, proof_type);
    
        assert(min_ind !=-1);
        // === Delete all children, except min_ind
        for(int i=0;i<node->children.size();i++){
            if( (node->children[i]==nullptr) || (min_ind == i)) continue;
            else{
                delete_all(node->children[i]);
                node->children[i]=nullptr;
            }
        }
    }
    return;
}

void PNS::free_states(){
    for(auto node_it: states){
        delete node_it.second;
    }
}

// ============================================
//                STOARING STATES
// ============================================
void PNS::add_board(const Board& board, PNS::PNSNode* node){
    #if ISOM
        std::vector<uint64_t> isom = isom_machine.get_canonical_graph(board, heuristic.all_linesinfo);
        assert(states.find(isom) == states.end());
        states[isom] = node;
    #else
        assert(states.find(board) == states.end());
        states[board] = node;
    #endif
}

PNS::PNSNode* PNS::get_states(const Board& board){
    #if ISOM
        std::vector<uint64_t> isom = isom_machine.get_canonical_graph(board, heuristic.all_linesinfo);
        //assert(states.find(isom) != states.end());
        if(states.find(isom) != states.end()){
            return states[isom];
        }
        else{
            return nullptr;
        }
    #else
        Board reversed(board);
        reversed.flip();

        if(states.find(board) != states.end()){
            return states[board];
        }
        else if(states.find(reversed) != states.end()){
            return states[reversed];
        }
        else{
            //assert(states.find(board) != states.end());
            return nullptr;
        }
    #endif
}

void PNS::delete_from_map(const Board& board){
    #if ISOM
        std::vector<uint64_t> isom = isom_machine.get_canonical_graph(board, heuristic.all_linesinfo);
        assert(states.find(isom) != states.end());
        states.erase(isom);
    #else
        assert(states.find(board) != states.end());
        states.erase(board);
    #endif
}

void PNS::display_node(PNSNode* node){
  display(node->board, true);
  std::cout<<std::endl;
  std::cout<<"Node type: "<<node->type<<" "<<node->pn<<" - "<<node->dn<<std::endl;
  if(!node->extended){
    std::cout<<"   leaf"<<std::endl;
  }
  else{
    for(int i=0;i<node->child_num;i++){
      if (node->children[i] == nullptr) {
        std::cout<<"Child: "<<i<<" nullptr"<<std::endl;
      }
      else{
        std::cout<<"Child: "<<i<< " dn: "<<node->children[i]->dn<< " pn: "<<node->children[i]->pn<< " extended: "<<node->children[i]->extended<<" heur val: "<<node->children[i]->heuristic_value<<std::endl;
      }
    }
  }
}

// ============================================
//                   STATS
// ============================================
void PNS::component_stats() {
  std::cout<< "COMPONENT CUT"<<std::endl;
  for (int depth=1; depth <= ACTION_SIZE; depth++) {
    int cnt = 0;
    int gainsum = 0;
    std::cout<<"Depth "<< depth <<": ";
    for (int j=0; j < ACTION_SIZE - depth; j++) {
      int freq = component_cut[ACTION_SIZE-depth][j];
      cnt += freq;
      gainsum += freq * j;
      std::cout<< freq << " ";
    }
    if (cnt > 0) {
        std::cout<<"---> "<< (float) gainsum / (float) cnt << std::endl;
      }
    else {
        std::cout<<"---> "<< 0 << std::endl;
    }
  }
}

void PNS::stats(PNSNode* node, bool end){
    if(node != nullptr) std::cout<<"\rPN: "<<node->pn<<" DN: "<<node->dn<<" ";
    std::cout<<"States size: "<<states.size()<<"        "<<std::flush;
    if(end) std::cout<<std::endl;
}