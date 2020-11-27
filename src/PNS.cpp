#include <fstream>

#include "PNS.h"
#include "limits.h"
#include "assert.h"
#include "common.h"
#include "artic_point.h"
#include "logger.h"
#include "counter.h"

#include <unistd.h>

PNS::PNSNode::PNSNode(const Board& b, Heuristic& h):children(), board(b){
    type = b.node_type;
    child_num = __builtin_popcountll(b.get_valids_without_ondegree(h.all_linesinfo));

    children.resize(child_num);
    heuristic_value = board.heuristic_value(h.all_linesinfo);
    
    int valid_moves = board.get_valids_num();
    float log_odds = -4.63230495 - 9.67572108 * board.node_type - 0.87216265 * valid_moves + 17.23691808 *heuristic_value;
    float prob = 1 / (1 + exp(-log_odds));
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
    //else if(prob<0.1){
    //    pn = var_MAX;
    //    dn = 0;
    //}
    else{
        init_pn_dn();
    }

    parent_num = 1;
}

void inline PNS::PNSNode::init_pn_dn(){
#if HEURISTIC
    // pn = 1/(1+exp(3*heuristic_value-2));
    int valid_moves = board.get_valids_num();
    // pn = std::pow(100, valid_moves / heuristic_value);
    // pn = std::pow(1, valid_moves - 15* heuristic_value);
    // float log_odds = -4.34417309 - 11.84912459 * board.node_type - 1.03582291 * valid_moves + 20.28989217 *heuristic_value;
    float log_odds = -4.63230495 - 9.67572108 * board.node_type - 0.87216265 * valid_moves + 17.23691808 *heuristic_value;
    float prob = 1 / (1 + exp(-log_odds));
    float penalty = (1-prob) * 10;
    pn = (type==OR ? 1+penalty:child_num*(1+penalty));
    // pn = (type==OR ? 1:child_num);

    // dn = 1/(1+exp(-3*heuristic_value-2));
    dn = std::pow(1000, heuristic_value);
    // float penalty_black = prob * 10;
    // dn = (type==AND ? 1+penalty_black:child_num*(1+penalty_black));
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

var PNS::get_min_children(PNS::PNSNode* node, const ProofType type) {
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

var PNS::get_sum_children(PNS::PNSNode* node, const ProofType type) {
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

PNS::PNSNode* PNS::create_and_eval_node(Board& board, bool eval){
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
    // int update_iteration = 1000;
    while(node->pn*node->dn != 0){
        PN_search(node, fast_eval);

        if(log && i%10000 == 0){
            stats(node);
        }
        // if(i == update_iteration){
        //     Counter counter;
        //     int update_count = counter.update_tree(node);
        //     update_iteration *= 2;
        //     Counter counter2;
        //     int node_count = counter2.count_nodes(node);
        //     std::cout<<"Iteration "<<i<<", updated: "<<update_count<<"/"<<node_count<<std::endl;
        // }
        ++i;
    }
}

void PNS::extend_all(PNS::PNSNode* node, bool fast_eval){
    // std::cout<<"extend_all"<<std::endl;
    if(node == nullptr) std::cout<<"died in extend_all"<<std::endl;
    if((node->pn == 0) || (node->dn == 0) || node->extended) return;
    int slot = 0;

    board_int valids = node->board.get_valids_without_ondegree(heuristic.all_linesinfo);
    for(int i=0;i<ACTION_SIZE;i++){
        if(valids & (1ULL << i)){
            extend(node, i, slot, fast_eval);
            // if(node->children[slot]->type == OR){
            //   extend_all(node->children[slot], fast_eval);
            // }
            if ((node->type == OR) and (node->children[slot]->pn==0) or
		(node->type == AND) and (node->children[slot]->dn==0)) break;
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

Board PNS::extend(PNS::PNSNode* node, unsigned int action, unsigned int slot,
		  bool fast_eval){
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
        int moves_before = next_state.get_valids_num();
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
        int moves_after = node->children[slot]->board.get_valids_num();
        
        component_cut[moves_before][moves_before - moves_after] += 1;

        return node->children[slot]->board;
    }
}

void PNS::init_PN_search(PNS::PNSNode* node){
    add_board(node->board, node);
}

void PNS::delete_and_log(PNS::PNSNode* node){
    #if LOG
    if(false && keep_only_one_child(node) && node->child_num > 0){
        int ind = rand() % node->child_num;
        if(node->children[ind] != nullptr && node->children[ind]->board.get_valids_num() < ROW*COL*0.75){
            evaluate_node_with_PNS(node->children[ind]);
            logger->log(node->children[ind], heuristic);
        }
    }
    #endif
    delete_node(node);
    #if LOG
    logger->log(node, heuristic);
    #endif
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
	if (min_ind != -1){
	    PN_search(node->children[min_ind], fast_eval);
	}
	update_node(node);
    }
    // If PN or DN is 0, delete all unused descendents
    if(node->pn == 0 || node->dn == 0){
        delete_and_log(node);
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
  if(node == nullptr) return;
  if( node->parent_num > 1 ){
    node->parent_num -= 1;
  }
  else{
    for(int i=0;i<node->child_num;i++){
      //          std::cout<<"child "<<i<<std::endl;
      delete_all(node->children[i]);
      node->children[i]=nullptr;
    }
    
    delete_from_map(node->board);
    delete node;
  }
}

  //We only keep a single child that proves the given node
void PNS::delete_node(PNS::PNSNode* node){
    // === In this case, we need max 1 branch ===
    // === DON'T DELETE THIS IF ===
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
//                STORING STATES
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
        total_state_size+=1;
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
    unsigned int x = states.size();
    float f = 1.0/(1+exp((args->A-x)/args->B));
    int N = (int) (states.size()*f);
    if(node != nullptr) std::cout<<"\rPN: "<<node->pn<<" DN: "<<node->dn<<" ";
    std::cout<<"States size: "<<states.size()<<" "<<N<<"        "<<std::flush;
    if(end) std::cout<<std::endl;
}
