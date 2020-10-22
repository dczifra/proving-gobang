#include <fstream>

#include "PNS.h"
#include "limits.h"
#include "assert.h"
#include "common.h"
#include "artic_point.h"

#include <unistd.h>

PNS::PNSNode::PNSNode(const Board& b, unsigned int d, Heuristic& h):children(), board(b), depth(d){
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

        pn_th = pn;
        dn_th = dn;
    }

    parent_num = 1;
}

void inline PNS::PNSNode::init_pn_dn(){
    #if HEURISTIC
  // double apn = 1/(1+exp(3*heuristic_value-2));
  // double adn = 1/(1+exp(-3*heuristic_value-2));
  int valid_moves = __builtin_popcountll(board.get_valids());
  double apn = std::pow(100, valid_moves / heuristic_value);
  double adn = std::pow(100, heuristic_value);
  pn = apn;
  dn = adn;
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

unsigned int PNS::get_min_children_index(PNS::PNSNode* node, const ProofType type) const{
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

// TODO this might need to be refactored
var PNS::get_min_delta_index(PNS::PNSNode* node, int& second_ind) const{
    var first_min = var_MAX;
    var second_min = var_MAX;

    unsigned int first_ind = -1;
    second_ind = -1;

    for(int i=0;i<node->child_num;i++){
        // === take care of initialization ===
        var child = node->children[i] == nullptr ? 1ULL: node->children[i]->delta();

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

var PNS::get_sum_children(PNS::PNSNode* node, const ProofType type) const{
  // var current_value = get_child_value(node, type);
  var sum = 0;

  for(int i=0;i<node->child_num;i++){
    if (node->children[i] == nullptr) continue;

    // #if HEURISTIC
    // // only update current value if all children are extended
    // if(!node->children[i]->extended) return current_value;
    // #endif
    
    var child = get_child_value(node->children[i], type);
    if(child == var_MAX) return var_MAX;
    sum += child;
  }
  return sum;
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
    if(next_state.node_type == OR){ // Last move: black
        next_state.remove_lines_with_two_ondegree(heuristic.all_linesinfo);
        next_state.remove_2lines_all(heuristic.all_linesinfo);
        next_state.remove_dead_fields_all(heuristic.all_linesinfo);
    }
}

PNS::PNSNode* PNS::create_and_eval_node(Board& board, int base_depth, bool eval, bool search = false){
    PNSNode* node;

    node = get_states(board);
    if(node != nullptr){
        node -> parent_num += 1;
    }
    else{
        node = new PNS::PNSNode(board, base_depth, heuristic);
        add_board(board, node);
    }

    if(eval){
      evaluate_node_with_PNS(node, false, false);
    }
    return node;
}

PNS::PNSNode* PNS::evaluate_components(Board& base_board, const int base_depth){
    assert(base_board.node_type == OR);
    bool delete_comps = true;

    int artic_point;
    Board small_board, big_board;
    Artic_point comps(base_board, heuristic.all_linesinfo, heuristic.linesinfo_per_field);
    std::tie(artic_point, small_board, big_board) = comps.get_parts();

    if(artic_point > -1){
        // 1. Evalute smaller component without artic point
        PNSNode* small_comp = create_and_eval_node(small_board, base_depth, true);
        #if TALKY
            display(small_board, true);
            std::cout<<small_comp->pn<<std::endl;
        #endif

        if(small_comp->pn == 0){
            #if TALKY
                std::cout<<"Small comp\n";
                display(small_board, true);
            #endif
            //std::cout<<" "<<__builtin_popcountll(small_board.get_valids())<<std::endl;
            return small_comp;
        }
        else{
          if (delete_comps) {
            delete_all(small_comp);
          }

            // 2. Evaluate small component with artic point
            (small_board.white) |= ((1ULL)<<artic_point);
            PNSNode* small_comp_mod = create_and_eval_node(small_board, base_depth, true);
            #if TALKY
                display(small_board, true);
                std::cout<<small_comp_mod->pn<<std::endl;
            #endif

            // 3-4. If Attacker wins: C* else C
            if(small_comp_mod->pn == 0){
                big_board.white |= ((1ULL)<<artic_point);
            }
            if (delete_comps){
              delete_all(small_comp_mod);
            }
            #if TALKY
                std::cout<<"Big comp\n";
                display(big_board, true);
            #endif

            // TODO: Improve...
            //bool eval_big = __builtin_popcountll(big_board.get_valids()) < EVAL_TRESHOLD;
            PNSNode* big_comp = create_and_eval_node(big_board, base_depth, false);
            return big_comp;
        }
    }
    else{
        PNSNode* node = new PNSNode(base_board, base_depth, heuristic);
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
      slot++;
    }
  }
  node->extended = true;

  // std::cout<<"extended"<<std::endl;
  // display_node(node);

#if HEURISTIC
  // default DN is not useful in OR nodes, so we update them
  // if(node->type == AND){
  //   double heur_parent = node->heuristic_value;
  //   double heur_min = DBL_MAX;
  //   for(int i=0;i<node->child_num;i++){
  //     if(node->children[i]->type == OR){
  //       heur_min = std::min(heur_min, node->children[i]->heuristic_value);
  //     }
  //   }
  //   for(int i=0;i<node->child_num;i++){
  //     PNSNode* current_child = node->children[i];
  //     if(current_child->type == OR){
  //       current_child->dn = std::pow(100, heur_parent - heur_min + current_child->heuristic_value);
  //     }
  //   }
  // }
    
    // if ((current_child->pn == 0) || (current_child->dn ==0) || current_child->extended) continue;  
    // if(node->type == OR){
    //   int valid_moves = __builtin_popcountll(current_child->board.get_valids());
    //   current_child->pn = valid_moves * std::pow(100, 1 / current_child->heuristic_value);
    // }

#endif

  update_node(node);
  // std::cout<<"updated"<<std::endl;
  // display_node(node);

}

Board PNS::extend(PNS::PNSNode* node, unsigned int action, unsigned int slot, bool fast_eval){
    Board next_state(node->board, action, get_player(node->type));

    simplify_board(next_state);
    while(!game_ended(next_state)){
      int temp_act = next_state.one_way(get_all_lines());
      if(temp_act > -1){
        next_state.move(temp_act, next_state.node_type== OR ? 1 : -1);
        simplify_board(next_state);
      }
      else break;
    }

    
    PNSNode* child = get_states(next_state);
    if(child != nullptr){
        node->children[slot] = child;
        node->children[slot] -> parent_num += 1;
        return next_state;
    }
    else{
        assert(node->children[slot] == nullptr);

        // monitor search space reduction due to the components
        int moves_before = __builtin_popcountll(next_state.get_valids());

        // 2-connected components, if not ended
        int valid_moves = __builtin_popcountll(next_state.get_valids());
        if(!fast_eval && next_state.node_type == OR && !game_ended(next_state)){ 
            node->children[slot] = evaluate_components(next_state, node->depth+1);
        } 
        else{
            node->children[slot] = new PNS::PNSNode(next_state, node->depth+1, heuristic);
            add_board(next_state, node->children[slot]);
            if(!fast_eval && valid_moves < EVAL_TRESHOLD){
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

void PNS::PN_search(PNS::PNSNode* node, bool fast_eval){
    assert(node != nullptr);
    // display_node(node);

    if(node->pn == 0 || node->dn == 0) return;

    // if we are in a leaf, we extend it
    if(!node->extended){
      extend_all(node, fast_eval);
    }
    else{
      unsigned int min_ind;
      if(node->type == OR) 
        min_ind = get_min_children_index(node, PN);
      else
        min_ind = get_min_children_index(node, DN);

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
  // std::cout<<"updating to:"<<node->pn<<" "<<node->dn<<std::endl;
  // display(node->board, true);
  // std::cout<<"node type: "<<node->type<<std::endl;
}
  

// void PNS::delete_children(PNS::PNSNode* node){
//     if( node->parent_num>1 ){
//         node->parent_num -= 1;
//     }
//     else{
//       for(int i=0;i<node->child_num;i++){
//         delete_all(node->children[i]);
//         node->children[i]=nullptr;
//       }
//     }
// }

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
  // std::cout<<node->type<<" "<<node->pn<<" "<<node->dn<<std::endl;
  // display(node->board, true);
  // assert(((node->type == OR) && (node->pn == 0)) || ((node->type == AND) && (node->dn==0)));
  assert((node->pn == 0) || (node->dn==0));
  assert(node != nullptr);
      
  ProofType proof_type = (node->pn == 0 ? PN:DN);
  unsigned int min_ind = get_min_children_index(node, proof_type);
  if (min_ind == -1) return;

  // === Delete all children, except min_ind
  for(int i=0;i<node->child_num;i++){
    if(min_ind == i) continue;

    delete_all(node->children[i]);
    node->children[i]=nullptr;
    // std::cout<<"www "<<i<<std::endl;
  }
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
      unsigned int min_ind = get_min_children_index(node, proof_type);
      if (min_ind == UINT_MAX) return;
      else log_solution_min(node->children[min_ind], file, logged);
    }
    else{
      for(int i=0;i<node->child_num;i++){
        log_solution_min(node->children[i], file, logged);
      }
    }
  }
}

void PNS::free_states(){
    for(auto node_it: states){
        delete node_it.second;
    }
}
/*
bool PNS::has_board(const Board& board){
    #if ISOM
        std::vector<uint64_t> isom = isom_machine.get_canonical_graph(board, heuristic.all_linesinfo);
        if(states.find(isom) != states.end()) return true;
        else return false;
    #else
        Board reversed(board);
        reversed.flip();

        if(states.find(board) != states.end()) return true;
        else if(states.find(reversed) != states.end()) return true;
        else return false;
    #endif
}*/

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
